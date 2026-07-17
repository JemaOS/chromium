// ===== État global + persistance localStorage =====

export const state = {
  agents: [],           // agents créés par l'utilisateur
  activeView: 'user',   // 'user' | agentId
  barHidden: false,
  orchMessages: [],     // historique du panneau orchestrateur
  settings: {
    provider: 'mistral',
    model: '',
    mock: false,
    configuredProviders: [],
  },
};

const STORAGE_KEY = 'jema-ai-env-proto-v1';

export function loadState() {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) return;
    const data = JSON.parse(raw);
    if (data.agents) state.agents = data.agents;
    if (data.settings) {
      delete data.settings.baseUrl;
      delete data.settings.proxyUrl;
      delete data.settings.apiKey;
      Object.assign(state.settings, data.settings);
      if (data.settings.apiKeyConfigured &&
          !state.settings.configuredProviders.includes(state.settings.provider)) {
        state.settings.configuredProviders.push(state.settings.provider);
      }
      delete state.settings.apiKeyConfigured;
    }
    if (data.orchMessages) state.orchMessages = data.orchMessages;
    // Les agents ne doivent pas rester "working" après un reload
    state.agents.forEach(agent => {
      if (agent.status === 'working') {
        agent.status = 'idle';
        agent.progress = 0;
      }
      // Migration v2 : les agents existants reçoivent les outils OS de base.
      const requiredTools = [
        'open_app', 'quicktext_open_content', 'get_active_window', 'open_url',
        'capture_active_window', 'get_accessibility_tree',
        'accessibility_action',
      ];
      if (agent.templateId === 'manager' || agent.templateId === 'custom') {
        requiredTools.push('change_language', 'list_display_modes', 'set_display_mode');
      }
      agent.tools = [...new Set([...(agent.tools || []), ...requiredTools])];
      agent.executionMode = agent.executionMode || 'ask';
    });
  } catch (e) {
    console.warn('État illisible, repart de zéro', e);
  }
}

export function saveState() {
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify({
      agents: state.agents,
      settings: state.settings,
      orchMessages: state.orchMessages.slice(-60),
    }));
  } catch (e) { /* quota */ }
  publishAgentState();
}

export function publishAgentState() {
  const agents = state.agents.map(agent => ({
    id: agent.id,
    name: agent.name,
    color: agent.color,
    progress: agent.progress || 0,
    active: state.activeView === agent.id,
    status: agent.status || 'idle',
    autonomous: agent.executionMode === 'autonomous',
  }));
  chrome.send('setJemaAgentState', [agents]);
}

export function uid() {
  return Date.now().toString(36) + Math.random().toString(36).slice(2, 7);
}

// ===== Agents =====

export function createAgent({ name, role, color, skills, systemPrompt, tools, templateId, executionMode = 'ask' }) {
  const agent = {
    id: uid(),
    name,
    role: role || 'Agent IA',
    color,
    letter: (name || 'A').trim().charAt(0).toUpperCase(),
    skills: skills || [],
    systemPrompt: systemPrompt || '',
    tools: tools || [],
    templateId: templateId || 'custom',
    executionMode,
    status: 'idle',      // idle | working | waiting | done
    progress: 0,
    items: [],           // livrables produits (notes, docs, tableaux…)
    activity: [],        // journal d'activité visible
    createdAt: Date.now(),
  };
  state.agents.push(agent);
  saveState();
  return agent;
}

export function getAgent(id) {
  return state.agents.find(a => a.id === id);
}

export function deleteAgent(id) {
  state.agents = state.agents.filter(a => a.id !== id);
  if (state.activeView === id) state.activeView = 'user';
  saveState();
}

export function setAgentStatus(id, status, progress) {
  const a = getAgent(id);
  if (!a) return;
  a.status = status;
  if (progress !== undefined) a.progress = Math.max(0, Math.min(100, progress));
  saveState();
}

export function addItem(agentId, item) {
  const a = getAgent(agentId);
  if (!a) return;
  a.items.push({ id: uid(), ts: Date.now(), ...item });
  saveState();
}

export function addActivity(agentId, text, icon) {
  const a = getAgent(agentId);
  if (!a) return;
  a.activity.push({ ts: Date.now(), text, icon: icon || '•' });
  if (a.activity.length > 250) a.activity.shift();
  saveState();
}

export function addOrchMessage(role, text) {
  state.orchMessages.push({ role, text, ts: Date.now() });
  if (state.orchMessages.length > 100) state.orchMessages.shift();
  saveState();
}
