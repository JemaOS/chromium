// ===== Abstraction multi-fournisseurs LLM =====
// Formats supportés : OpenAI-compatible (Mistral, Kimi, GLM) + Anthropic (Claude)

import { state } from './state.js';
import {sendWithPromise} from 'chrome://resources/ash/common/cr.m.js';

export const PROVIDERS = {
  mistral: {
    label: 'Mistral',
    baseUrl: 'https://api.mistral.ai/v1/chat/completions',
    models: ['mistral-large-latest', 'mistral-small-latest', 'codestral-latest'],
    format: 'openai',
  },
  kimi: {
    label: 'Kimi (Moonshot)',
    baseUrl: 'https://api.moonshot.ai/v1/chat/completions',
    models: ['kimi-k2-0905-preview', 'kimi-k2-turbo-preview', 'kimi-latest'],
    format: 'openai',
  },
  glm: {
    label: 'GLM (Zhipu)',
    baseUrl: 'https://open.bigmodel.cn/api/paas/v4/chat/completions',
    models: ['glm-4.6', 'glm-4.5', 'glm-4.5-air'],
    format: 'openai',
  },
  claude: {
    label: 'Claude (Anthropic)',
    baseUrl: 'https://api.anthropic.com/v1/messages',
    models: ['claude-sonnet-4-5', 'claude-haiku-4-5', 'claude-opus-4-5'],
    format: 'anthropic',
  },
};

// Format interne des messages : style OpenAI
// { role, content, tool_calls?: [{id, type:'function', function:{name, arguments}}], tool_call_id?, name? }
// Retour unifié : { text: string, toolCalls: [{id, name, args}] }

export async function chatCompletion({ system, messages, tools }) {
  if (state.settings.mock) {
    return mockCompletion({ system, messages, tools });
  }
  const provider = PROVIDERS[state.settings.provider];
  if (!provider) throw new Error(`Fournisseur inconnu: ${state.settings.provider}`);
  const secret = await sendWithPromise('loadJemaApiKey', state.settings.provider);
  if (!secret.apiKey) throw new Error('Clé API manquante — ouvrez les réglages ⚙');

  if (provider.format === 'anthropic') {
    return callAnthropic({ provider, system, messages, tools, apiKey: secret.apiKey });
  }
  return callOpenAI({ provider, system, messages, tools, apiKey: secret.apiKey });
}

async function callOpenAI({ provider, system, messages, tools, apiKey }) {
  const url = provider.baseUrl;
  const body = {
    model: state.settings.model || provider.models[0],
    messages: (system ? [{ role: 'system', content: system }, ...messages] : messages)
        .map(toOpenAIMessage),
    temperature: 0.4,
  };
  if (tools?.length) {
    body.tools = tools;
    body.tool_choice = 'auto';
  }

  const res = await nativeLlmRequest(url, provider.format, body, apiKey);
  if (res.status < 200 || res.status >= 300) {
    throw new Error(`API ${res.status}: ${res.body.slice(0, 200)}`);
  }
  const data = JSON.parse(res.body);
  const msg = data.choices?.[0]?.message || {};
  return {
    text: msg.content || '',
    toolCalls: (msg.tool_calls || []).map(tc => ({
      id: tc.id,
      name: tc.function?.name,
      args: safeParseArgs(tc.function?.arguments),
    })),
  };
}

function toOpenAIMessage(message) {
  if (!Array.isArray(message.content)) return message;
  return {
    ...message,
    content: message.content.map(part => part.type === 'image' ? {
      type: 'image_url',
      image_url: {url: `data:${part.mimeType};base64,${part.data}`},
    } : part),
  };
}

async function callAnthropic({ provider, system, messages, tools, apiKey }) {
  const url = provider.baseUrl;
  const body = {
    model: state.settings.model || provider.models[0],
    max_tokens: 4096,
    messages: toAnthropicMessages(messages),
  };
  if (system) body.system = system;
  if (tools?.length) {
    body.tools = tools.map(t => ({
      name: t.function.name,
      description: t.function.description,
      input_schema: t.function.parameters,
    }));
  }

  const res = await nativeLlmRequest(url, provider.format, body, apiKey);
  if (res.status < 200 || res.status >= 300) {
    throw new Error(`Anthropic ${res.status}: ${res.body.slice(0, 200)}`);
  }
  const data = JSON.parse(res.body);
  const textParts = [];
  const toolCalls = [];
  for (const block of data.content || []) {
    if (block.type === 'text') textParts.push(block.text);
    if (block.type === 'tool_use') toolCalls.push({ id: block.id, name: block.name, args: block.input || {} });
  }
  return { text: textParts.join('\n'), toolCalls };
}

// Convertit le format OpenAI interne → messages Anthropic
function toAnthropicMessages(messages) {
  const out = [];
  let pendingToolResults = [];

  const flushToolResults = () => {
    if (pendingToolResults.length) {
      out.push({ role: 'user', content: pendingToolResults });
      pendingToolResults = [];
    }
  };

  for (const m of messages) {
    if (m.role === 'tool') {
      pendingToolResults.push({
        type: 'tool_result',
        tool_use_id: m.tool_call_id,
        content: typeof m.content === 'string' ? m.content : JSON.stringify(m.content),
      });
      continue;
    }
    flushToolResults();
    if (m.role === 'assistant') {
      const content = [];
      if (m.content) content.push({ type: 'text', text: m.content });
      for (const tc of m.tool_calls || []) {
        content.push({
          type: 'tool_use',
          id: tc.id,
          name: tc.function.name,
          input: safeParseArgs(tc.function.arguments),
        });
      }
      out.push({ role: 'assistant', content: content.length ? content : [{ type: 'text', text: '…' }] });
    } else if (m.role === 'user') {
      const content = Array.isArray(m.content) ? m.content.map(part =>
        part.type === 'image' ? {
          type: 'image',
          source: {type: 'base64', media_type: part.mimeType, data: part.data},
        } : part) : m.content;
      out.push({ role: 'user', content });
    }
  }
  flushToolResults();
  return out;
}

function safeParseArgs(raw) {
  if (!raw) return {};
  if (typeof raw === 'object') return raw;
  try { return JSON.parse(raw); } catch { return {}; }
}

async function nativeLlmRequest(endpoint, format, body, apiKey) {
  const response = await sendWithPromise('llmRequest', {
    endpoint,
    format,
    apiKey,
    body: JSON.stringify(body),
  });
  if (response.netError !== 0 && response.status === 0) {
    throw new Error(`Erreur réseau ${response.netError}`);
  }
  return response;
}

// ===== Mode simulation (sans clé API) =====

async function mockCompletion({ system, messages, tools }) {
  await new Promise(r => setTimeout(r, 600 + Math.random() * 600));
  const toolNames = (tools || []).map(t => t.function?.name);

  // Orchestrateur ?
  if (toolNames.includes('delegate_to_agent')) {
    const userText = [...messages].reverse().find(m => m.role === 'user')?.content || '';
    return mockOrchestrate(userText);
  }

  // Agent : scénario en 3 étapes selon l'avancement (nb de messages tool)
  const toolMsgCount = messages.filter(m => m.role === 'tool').length;
  const task = messages.find(m => m.role === 'user')?.content || 'la tâche';

  if (toolMsgCount === 0) {
    return {
      text: `Compris. Je m'occupe de « ${task.slice(0, 80)} ». Je commence par structurer le travail.`,
      toolCalls: [
        { id: 'mock-1', name: pickTool(toolNames, 'create_note'), args: { title: 'Plan de travail', content: `Tâche : ${task}\n\n1. Analyse\n2. Production\n3. Vérification` } },
        { id: 'mock-2', name: 'report_progress', args: { percent: 30, step: 'Plan structuré' } },
      ].filter(tc => toolNames.includes(tc.name) || tc.name === 'report_progress' || tc.name === 'task_complete'),
    };
  }
  if (toolMsgCount <= 2) {
    const calls = [];
    if (toolNames.includes('calculate')) {
      calls.push({ id: 'mock-3', name: 'calculate', args: { expression: '(245.67 * 8.94) + (156.78 / 3.2)' } });
    }
    if (toolNames.includes('create_document')) {
      calls.push({ id: 'mock-4', name: 'create_document', args: { title: 'Livrable', content: `# Résultat\n\nDocument produit pour : ${task.slice(0, 60)}\n\nContenu de démonstration généré en mode simulation.` } });
    }
    calls.push({ id: 'mock-5', name: 'report_progress', args: { percent: 75, step: 'Production en cours' } });
    return { text: 'Je produis les livrables.', toolCalls: calls };
  }
  return {
    text: 'Travail terminé, je clos la tâche.',
    toolCalls: [{ id: 'mock-6', name: 'task_complete', args: { summary: `« ${task.slice(0, 60)} » traitée avec succès (simulation).` } }],
  };
}

function mockOrchestrate(userText) {
  const agents = state.agents;
  if (!agents.length) {
    return { text: '', toolCalls: [{ id: 'mock-o1', name: 'respond', args: { message: "Créez d'abord un agent avec le bouton + pour que je puisse déléguer." } }] };
  }
  const lower = userText.toLowerCase();
  const match = agents.find(a =>
    a.name.toLowerCase().split(/\s+/).some(w => w && lower.includes(w)) ||
    a.role.toLowerCase().split(/\s+/).some(w => w.length > 4 && lower.includes(w)) ||
    a.skills.some(s => lower.includes(s.toLowerCase().split(' ')[0]))
  ) || agents[0];
  return {
    text: '',
    toolCalls: [{ id: 'mock-o2', name: 'delegate_to_agent', args: { agent_id: match.id, task: userText } }],
  };
}

function pickTool(toolNames, wanted) {
  return toolNames.includes(wanted) ? wanted : (toolNames.find(n => n.startsWith('create_')) || wanted);
}
