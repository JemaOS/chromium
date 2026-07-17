// ===== Orchestrateur + boucle agentique =====

import { state, getAgent, setAgentStatus, addActivity, addOrchMessage } from './state.js';
import { chatCompletion } from './llm.js';
import { schemasFor, executeTool, toolIcon, toolLabel } from './tools.js';

// Callbacks UI injectés par main.js (évite les imports circulaires)
let ui = {};
export function bindUI(callbacks) { ui = callbacks; }

const ORCHESTRATOR_TOOLS = [
  {
    type: 'function',
    function: {
      name: 'delegate_to_agent',
      description: "Déléguer la tâche à un agent spécialisé qui l'exécutera dans son espace de travail",
      parameters: {
        type: 'object',
        properties: {
          agent_id: { type: 'string', description: "ID de l'agent choisi" },
          task: { type: 'string', description: 'Tâche complète et claire à confier à l\'agent' },
        },
        required: ['agent_id', 'task'],
      },
    },
  },
  {
    type: 'function',
    function: {
      name: 'respond',
      description: "Répondre directement à l'utilisateur sans déléguer (question simple, conversation)",
      parameters: {
        type: 'object',
        properties: { message: { type: 'string' } },
        required: ['message'],
      },
    },
  },
];

function buildOrchestratorPrompt() {
  const agentList = state.agents.map(a => ({
    id: a.id,
    nom: a.name,
    role: a.role,
    skills: a.skills,
  }));
  return `Tu es l'orchestrateur de JemaOS. Tu reçois les demandes de l'utilisateur et décides :
- soit DÉLÉGUER à un agent spécialisé via delegate_to_agent (toute tâche de production : rédaction, calcul, tableau, analyse, code, recherche…),
- soit RÉPONDRE directement via respond (question simple, salutation, demande d'information sur les agents).

Agents disponibles :
${JSON.stringify(agentList, null, 2)}

Règles :
- Choisis l'agent le plus pertinent selon ses skills.
- Reformule la tâche déléguée de manière complète et autonome (l'agent ne voit que ce texte).
- Si la demande mentionne plusieurs tâches, délègue la principale au meilleur agent.
- Réponds en français.`;
}

export async function orchestrate(userText) {
  addOrchMessage('user', userText);
  ui.renderOrchMessages?.();
  ui.setOrchestratorThinking?.(true);

  try {
    const res = await chatCompletion({
      system: buildOrchestratorPrompt(),
      messages: [{ role: 'user', content: userText }],
      tools: ORCHESTRATOR_TOOLS,
    });

    const call = res.toolCalls?.[0];

    if (call?.name === 'delegate_to_agent') {
      const agent = getAgent(call.args.agent_id) || state.agents[0];
      if (!agent) {
        addOrchMessage('system', "Aucun agent disponible. Créez-en un avec le bouton +.");
      } else {
        addOrchMessage('system', `🎯 Délégué à ${agent.name} — « ${call.args.task} »`);
        ui.renderOrchMessages?.();
        // L'agent démarre en arrière-plan ; on ne bloque pas le panneau
        runAgentTask(agent, call.args.task);
        return;
      }
    } else if (call?.name === 'respond') {
      addOrchMessage('assistant', call.args.message);
    } else if (res.text) {
      addOrchMessage('assistant', res.text);
    } else {
      addOrchMessage('assistant', "Je n'ai pas compris la demande.");
    }
  } catch (e) {
    addOrchMessage('system', `❌ ${e.message}`);
  } finally {
    ui.setOrchestratorThinking?.(false);
    ui.renderOrchMessages?.();
  }
}

export async function runAgentTask(agent, task) {
  setAgentStatus(agent.id, 'working', 5);
  addActivity(agent.id, `Nouvelle tâche : ${task}`, '🎯');
  ui.toast?.(`${agent.name} commence à travailler`, agent.color);
  ui.renderTopBar?.();
  ui.switchView?.(agent.id); // on le voit travailler dans son desk

  const systemPrompt = `${agent.systemPrompt}

Contexte technique : tu travailles dans ton espace de travail dédié de JemaOS. Tout ce que tu produis avec tes outils est visible par l'utilisateur en temps réel. Sois concret : chaque livrable doit être créé avec un outil, pas seulement décrit.
Pour contrôler une interface, suis toujours cette boucle : observer (capture ou arbre accessible), planifier, agir avec un token, vérifier l'état renvoyé après l'action, puis corriger si nécessaire. N'invente jamais un token et n'utilise jamais un token après navigation.`;

  const messages = [{ role: 'user', content: task }];
  const tools = schemasFor(agent.tools);
  let done = false;
  let iterations = 0;
  const MAX_ITER = 12;
  const repeatedCalls = new Map();

  while (!done && iterations < MAX_ITER) {
    iterations++;
    let res;
    try {
      res = await chatCompletion({ system: systemPrompt, messages, tools });
    } catch (e) {
      addActivity(agent.id, `Erreur API : ${e.message}`, '❌');
      setAgentStatus(agent.id, 'idle');
      ui.toast?.(`${agent.name} : erreur API`, '#ef4444');
      ui.renderTopBar?.();
      ui.renderWorkspace?.();
      return;
    }

    // Empile le message assistant au format interne (OpenAI-like)
    const assistantMsg = { role: 'assistant', content: res.text || '' };
    if (res.toolCalls?.length) {
      assistantMsg.tool_calls = res.toolCalls.map(tc => ({
        id: tc.id,
        type: 'function',
        function: { name: tc.name, arguments: JSON.stringify(tc.args) },
      }));
    }
    messages.push(assistantMsg);

    if (res.text) {
      addActivity(agent.id, res.text, '💬');
      ui.renderWorkspace?.();
    }

    if (!res.toolCalls?.length) break;

    for (const tc of res.toolCalls) {
      const callSignature = `${tc.name}:${JSON.stringify(tc.args)}`;
      const repeated = (repeatedCalls.get(callSignature) || 0) + 1;
      repeatedCalls.set(callSignature, repeated);
      if (repeated > 2) {
        addActivity(agent.id, `Action répétée bloquée : ${tc.name}`, '⛔');
        messages.push({
          role: 'tool', tool_call_id: tc.id, name: tc.name,
          content: JSON.stringify({ok: false, error: 'Repeated action blocked. Observe again and choose a different action.'}),
        });
        continue;
      }
      addActivity(agent.id, activityLabel(tc), toolIcon(tc.name));
      ui.renderWorkspace?.();

      let result;
      try {
        result = await executeTool(agent, tc.name, tc.args);
      } catch (e) {
        result = { ok: false, error: e.message };
        addActivity(agent.id, `Erreur outil ${toolLabel(tc.name)} : ${e.message}`, '⚠️');
      }

      const toolResult = tc.name === 'capture_active_window' && result?.base64 ?
          {...result, base64: '[transmis comme image multimodale]'} : result;
      messages.push({
        role: 'tool',
        tool_call_id: tc.id,
        name: tc.name,
        content: JSON.stringify(toolResult),
      });
      if (tc.name === 'capture_active_window' && result?.base64) {
        messages.push({
          role: 'user',
          content: [
            {type: 'text', text: 'Analyse cette capture de la fenêtre utilisateur active.'},
            {type: 'image', mimeType: result.mimeType || 'image/png', data: result.base64},
          ],
        });
      }

      if (tc.name === 'report_progress') {
        setAgentStatus(agent.id, 'working', tc.args.percent);
        ui.renderTopBar?.();
      }
      if (tc.name === 'notify_user' && result?.notify) {
        ui.toast?.(`🔔 ${result.notify}`, agent.color);
      }
      if (tc.name === 'task_complete') done = true;

      ui.renderWorkspace?.();
      await sleep(350); // tempo pour voir l'agent travailler
    }
  }

  if (!done) {
    addActivity(agent.id, "Tâche interrompue (limite d'étapes atteinte)", '⏸️');
    setAgentStatus(agent.id, 'idle');
  } else {
    addActivity(agent.id, 'Tâche terminée', '✅');
    setAgentStatus(agent.id, 'done', 100);
    ui.toast?.(`${agent.name} a terminé sa tâche ✅`, agent.color);
  }
  ui.renderTopBar?.();
  ui.renderWorkspace?.();
}

function activityLabel(tc) {
  const a = tc.args || {};
  switch (tc.name) {
    case 'create_note': return `Création de la note « ${a.title} »`;
    case 'create_document': return `Rédaction du document « ${a.title} »`;
    case 'create_spreadsheet': return `Construction du tableau « ${a.title} »`;
    case 'calculate': return `Calcul : ${a.expression}`;
    case 'web_search': return `Recherche : ${a.query}`;
    case 'notify_user': return `Notification envoyée`;
    case 'open_app': return `Lancement de l'application « ${a.app} »`;
    case 'quicktext_open_content': return `Ouverture de « ${a.title || 'Document'} » dans QuickText`;
    case 'change_language': return `Changement de la langue JemaOS vers ${a.locale}`;
    case 'list_display_modes': return `Lecture des modes d'affichage disponibles`;
    case 'set_display_mode': return `Changement de résolution vers ${a.description || `le mode ${a.modeIndex}`}`;
    case 'report_progress': return a.step ? `Progression ${a.percent}% — ${a.step}` : `Progression ${a.percent}%`;
    case 'task_complete': return `Clôture : ${a.summary}`;
    default: return tc.name;
  }
}

function sleep(ms) { return new Promise(r => setTimeout(r, ms)); }
