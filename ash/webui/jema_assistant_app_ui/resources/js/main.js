// ===== Point d'entrée : câblage des événements =====

import { state, loadState, saveState, createAgent, publishAgentState } from './state.js';
import {addWebUIListener} from 'chrome://resources/ash/common/cr.m.js';
import { PROVIDERS, chatCompletion } from './llm.js';
import { AGENT_TEMPLATES, PALETTE } from './templates.js';
import { orchestrate, bindUI } from './orchestrator.js';
import {executeSystemTool} from './tools.js';
import * as ui from './ui.js';

const $ = sel => document.querySelector(sel);

let selectedTemplate = null;
let selectedColor = PALETTE[0];

// ===== Init =====

loadState();
chrome.send('onJemaAssistantSwaInit');

// Premier démarrage : un agent système permet à l'orchestrateur d'exécuter
// immédiatement les demandes générales et les réglages JemaOS.
if (!state.agents.length) {
  const manager = AGENT_TEMPLATES.find(template => template.id === 'manager');
  createAgent({
    name: manager.name,
    role: manager.role,
    color: manager.color,
    skills: manager.skills,
    systemPrompt: manager.systemPrompt,
    tools: manager.tools,
    templateId: manager.id,
    executionMode: 'ask',
  });
}

bindUI({
  toast: ui.toast,
  renderTopBar: ui.renderTopBar,
  renderWorkspace: ui.renderWorkspace,
  renderOrchMessages: ui.renderOrchMessages,
  setOrchestratorThinking: ui.setOrchestratorThinking,
  switchView: ui.switchView,
});

ui.renderTopBar();
ui.renderWorkspace();
ui.renderOrchMessages();
ui.updateProviderLabel();
publishAgentState();

// Bridge direct Ash <-> WebUI. Aucun iframe ni frontend distant.
addWebUIListener('create-agent-requested', () => openAgentModal());
addWebUIListener('voice-input-requested', () => startVoiceInput());
addWebUIListener('agent-activated', agentId => {
  const agent = state.agents.find(a => a.id === agentId);
  if (agent) ui.switchView(agent.id);
});
addWebUIListener('desk-agent-activated', agentId => {
  const agent = state.agents.find(a => a.id === agentId);
  if (agent) ui.switchView(agent.id);
});

const source = new URLSearchParams(window.location.search).get('source');
if (source === 'bubble') {
  document.body.classList.add('native-bubble');
  $('#orch-panel').classList.remove('hidden');
}

// ===== Barre haute =====

$('#toggle-bar-btn').addEventListener('click', () => {
  state.barHidden = true;
  document.body.classList.add('bar-hidden');
  $('#orch-panel').classList.add('hidden');
  $('#show-bar-btn').classList.remove('hidden');
  ui.switchView('user'); // retour à l'environnement utilisateur
});

$('#show-bar-btn').addEventListener('click', () => {
  state.barHidden = false;
  document.body.classList.remove('bar-hidden');
  $('#show-bar-btn').classList.add('hidden');
});

$('#home-btn').addEventListener('click', () => ui.switchView('user'));

// ===== Orchestrateur =====

$('#orchestrator-btn').addEventListener('click', () => {
  ui.closeModal('#agent-modal');
  ui.closeModal('#settings-modal');
  $('#orch-panel').classList.toggle('hidden');
  if (!$('#orch-panel').classList.contains('hidden')) {
    ui.updateProviderLabel();
    ui.renderOrchMessages();
    $('#orch-input').focus();
  }
});

$('#orch-settings').addEventListener('click', openSettings);
$('#orch-close').addEventListener('click', () => {
  if (source === 'bubble') {
    chrome.send('onCloseAssistant', ['bubble']);
  } else {
    $('#orch-panel').classList.add('hidden');
  }
});

$('#orch-form').addEventListener('submit', e => {
  e.preventDefault();
  const input = $('#orch-input');
  const text = input.value.trim();
  if (!text) return;
  input.value = '';

  orchestrate(text);
});

// ===== Modal création d'agent =====

$('#add-agent-btn').addEventListener('click', openAgentModal);
$('#agent-close').addEventListener('click', closeAgentModal);
$('#agent-cancel').addEventListener('click', closeAgentModal);

function openAgentModal() {
  $('#orch-panel').classList.add('hidden');
  ui.closeModal('#settings-modal');
  renderTemplateGallery();
  renderColorPalette();
  selectTemplate(AGENT_TEMPLATES[0]);
  ui.openModal('#agent-modal');
}

function closeAgentModal() {
  ui.closeModal('#agent-modal');
  if (source === 'bubble') $('#orch-panel').classList.remove('hidden');
}

function renderTemplateGallery() {
  const gallery = $('#template-gallery');
  gallery.innerHTML = '';
  for (const tpl of AGENT_TEMPLATES) {
    const card = document.createElement('div');
    card.className = 'tpl-card';
    card.style.setProperty('--tpl-color', tpl.color);
    card.dataset.tpl = tpl.id;
    card.innerHTML = `
      <div class="tpl-head">
        <div class="tpl-dot" style="background:${tpl.color}">${tpl.name.charAt(0)}</div>
        <div class="tpl-name">${tpl.name}</div>
      </div>
      <div class="tpl-role">${tpl.role}</div>
      <div class="chips">${tpl.skills.slice(0, 3).map(s => `<span class="chip">${s}</span>`).join('')}</div>
    `;
    card.addEventListener('click', () => selectTemplate(tpl));
    gallery.appendChild(card);
  }
}

function selectTemplate(tpl) {
  selectedTemplate = tpl;
  document.querySelectorAll('.tpl-card').forEach(c =>
    c.classList.toggle('selected', c.dataset.tpl === tpl.id));
  $('#agent-name').value = tpl.id === 'custom' ? '' : tpl.name;
  $('#agent-role').value = tpl.id === 'custom' ? '' : tpl.role;
  $('#agent-sysprompt').value = tpl.systemPrompt;
  $('#agent-execution-mode').value = 'ask';
  $('#agent-skills-preview').innerHTML = tpl.skills.length
    ? tpl.skills.map(s => `<span class="chip on" style="--agent-color:${tpl.color}">${s}</span>`).join('')
    : '<span class="chip">Définissez les skills dans le system prompt</span>';
  selectColor(tpl.color);
  ui.renderToolCheckboxes(tpl.tools);
}

function renderColorPalette() {
  const box = $('#color-palette');
  box.innerHTML = '';
  for (const color of PALETTE) {
    const sw = document.createElement('button');
    sw.type = 'button';
    sw.className = 'color-swatch';
    sw.style.background = color;
    sw.dataset.color = color;
    sw.addEventListener('click', () => selectColor(color));
    box.appendChild(sw);
  }
}

function selectColor(color) {
  selectedColor = color;
  document.querySelectorAll('.color-swatch').forEach(s =>
    s.classList.toggle('selected', s.dataset.color === color));
}

$('#agent-form').addEventListener('submit', e => {
  e.preventDefault();
  const name = $('#agent-name').value.trim();
  if (!name) return;

  const agent = createAgent({
    name,
    role: $('#agent-role').value.trim() || selectedTemplate.role,
    color: selectedColor,
    skills: selectedTemplate.id === 'custom'
      ? $('#agent-role').value.trim() ? [$('#agent-role').value.trim()] : ['Polyvalent']
      : selectedTemplate.skills,
    systemPrompt: $('#agent-sysprompt').value.trim() || selectedTemplate.systemPrompt,
    tools: ui.getCheckedTools(),
    templateId: selectedTemplate.id,
    executionMode: $('#agent-execution-mode').value,
  });

  executeSystemTool(agent, {
    tool: 'permissions.set',
    targetAgentId: agent.id,
    targetTool: '__mode__',
    value: agent.executionMode === 'autonomous' ? 'allow' :
        agent.executionMode === 'blocked' ? 'deny' : 'ask',
    confirmed: true,
  }).catch(() => {});
  for (const tool of agent.tools) {
    const nativeTool = {
      open_app: 'apps.launch',
      quicktext_open_content: 'quicktext.open_content',
      change_language: 'locale.change',
      list_display_modes: 'display.list_modes',
      set_display_mode: 'display.set_mode',
      search_files: 'files.search',
      read_file: 'files.read',
      write_file: 'files.write',
      get_audio: 'audio.get',
      set_audio: 'audio.set',
      get_power: 'power.get',
      native_notification: 'notifications.show',
      compose_email: 'email.compose',
      get_wifi: 'wifi.get',
      set_wifi: 'wifi.set_enabled',
      get_bluetooth: 'bluetooth.get',
      set_bluetooth: 'bluetooth.set_enabled',
      get_active_window: 'environment.get_active_window',
      open_url: 'browser.open_url',
      capture_active_window: 'environment.capture_active_window',
      get_accessibility_tree: 'accessibility.get_active_tree',
      accessibility_action: 'accessibility.perform_action',
    }[tool];
    if (!nativeTool) continue;
    executeSystemTool(agent, {
      tool: 'permissions.set',
      targetAgentId: agent.id,
      targetTool: nativeTool,
      value: agent.executionMode === 'blocked' ? 'deny' :
          agent.executionMode === 'autonomous' ? 'allow' : 'ask',
      confirmed: true,
    }).catch(() => {});
  }

  ui.closeModal('#agent-modal');
  ui.renderTopBar();
  ui.renderWorkspace();
  ui.toast(`Agent « ${agent.name} » créé — son desk est prêt`, agent.color);
  ui.switchView(agent.id);
});

// ===== Modal réglages =====

$('#settings-btn').addEventListener('click', openSettings);
$('#settings-cancel').addEventListener('click', closeSettingsModal);
$('#settings-close').addEventListener('click', closeSettingsModal);

async function openSettings() {
  ui.closeModal('#agent-modal');
  $('#orch-panel').classList.add('hidden');
  const providerSelect = $('#set-provider');
  providerSelect.innerHTML = Object.entries(PROVIDERS)
    .map(([id, p]) => `<option value="${id}"${id === state.settings.provider ? ' selected' : ''}>${p.label}</option>`)
    .join('');
  fillProviderDefaults();
  $('#set-apikey').value = '';
  $('#set-mock').checked = state.settings.mock;
  await refreshProviderSecretStatus();
  ui.openModal('#settings-modal');
}

function closeSettingsModal() {
  ui.closeModal('#settings-modal');
  if (source === 'bubble') $('#orch-panel').classList.remove('hidden');
}

let speechRecognition = null;

function startVoiceInput() {
  $('#orch-panel').classList.remove('hidden');
  ui.closeModal('#agent-modal');
  ui.closeModal('#settings-modal');
  ui.updateProviderLabel();
  const Recognition = window.SpeechRecognition || window.webkitSpeechRecognition;
  if (!Recognition) {
    ui.toast('La dictée vocale n’est pas disponible sur cette configuration.', '#ef4444');
    return;
  }
  if (speechRecognition) speechRecognition.abort();
  const input = $('#orch-input');
  speechRecognition = new Recognition();
  speechRecognition.lang = navigator.language || 'fr-FR';
  speechRecognition.interimResults = true;
  speechRecognition.continuous = false;
  speechRecognition.onstart = () => {
    document.body.classList.add('voice-listening');
    input.placeholder = 'Écoute en cours…';
  };
  speechRecognition.onresult = event => {
    let transcript = '';
    let finalTranscript = '';
    for (let index = event.resultIndex; index < event.results.length; ++index) {
      transcript += event.results[index][0].transcript;
      if (event.results[index].isFinal) finalTranscript += event.results[index][0].transcript;
    }
    input.value = transcript;
    if (finalTranscript.trim()) {
      input.value = '';
      orchestrate(finalTranscript.trim());
    }
  };
  speechRecognition.onerror = event => {
    ui.toast(`Dictée vocale : ${event.error}`, '#ef4444');
  };
  speechRecognition.onend = () => {
    document.body.classList.remove('voice-listening');
    input.placeholder = 'Demandez à l’orchestrateur…';
    speechRecognition = null;
  };
  speechRecognition.start();
}

function fillProviderDefaults() {
  const p = PROVIDERS[$('#set-provider').value];
  $('#set-model').value = state.settings.model || p.models[0];
  $('#model-suggestions').innerHTML = p.models.map(m => `<option value="${m}">`).join('');
}

function setSettingsStatus(message, type = '') {
  const status = $('#settings-status');
  status.textContent = message;
  status.className = `settings-status ${type}`.trim();
}

async function refreshProviderSecretStatus() {
  const provider = $('#set-provider').value;
  try {
    const secret = await sendWithPromise('loadJemaApiKey', provider);
    if (secret.apiKey) {
      if (!state.settings.configuredProviders.includes(provider)) {
        state.settings.configuredProviders.push(provider);
        saveState();
      }
      setSettingsStatus(`Une clé chiffrée est configurée pour ${provider}.`, 'success');
      return true;
    }
  } catch (_) {}
  state.settings.configuredProviders =
      state.settings.configuredProviders.filter(item => item !== provider);
  saveState();
  setSettingsStatus(`Aucune clé configurée pour ${provider}.`, '');
  return false;
}

$('#set-provider').addEventListener('change', () => {
  state.settings.model = '';
  fillProviderDefaults();
  refreshProviderSecretStatus();
});

$('#settings-form').addEventListener('submit', async e => {
  e.preventDefault();
  const button = $('#settings-save');
  button.disabled = true;
  setSettingsStatus('Enregistrement sécurisé…', 'busy');
  try {
    state.settings.provider = $('#set-provider').value;
    state.settings.model = $('#set-model').value.trim();
    state.settings.mock = $('#set-mock').checked;
    const apiKey = $('#set-apikey').value.trim();
    const configured = state.settings.configuredProviders.includes(state.settings.provider);
    if (!state.settings.mock && !apiKey && !configured) {
      throw new Error('Saisissez une clé API ou activez le mode simulation.');
    }
    if (apiKey) {
      await sendWithPromise('saveJemaApiKey', state.settings.provider, apiKey);
      const check = await sendWithPromise('loadJemaApiKey', state.settings.provider);
      if (!check.apiKey) throw new Error('La clé n’a pas pu être relue après chiffrement.');
      if (!state.settings.configuredProviders.includes(state.settings.provider)) {
        state.settings.configuredProviders.push(state.settings.provider);
      }
      $('#set-apikey').value = '';
    }
    saveState();
    ui.updateProviderLabel();
    setSettingsStatus('Réglages enregistrés et clé chiffrée.', 'success');
    ui.toast('Réglages enregistrés', '#00cc66');
    setTimeout(closeSettingsModal, 500);
  } catch (error) {
    setSettingsStatus(`Échec : ${error.message || error}`, 'error');
  } finally {
    button.disabled = false;
  }
});

$('#settings-test').addEventListener('click', async () => {
  const btn = $('#settings-test');
  btn.disabled = true;
  btn.textContent = 'Test en cours…';
  setSettingsStatus('Test du stockage sécurisé et du fournisseur…', 'busy');
  // Applique temporairement les valeurs du formulaire pour le test
  const backup = { ...state.settings };
  state.settings.provider = $('#set-provider').value;
  state.settings.model = $('#set-model').value.trim();
  try {
    const testApiKey = $('#set-apikey').value.trim();
    state.settings.mock = $('#set-mock').checked;
    const configured = state.settings.configuredProviders.includes(state.settings.provider);
    if (!state.settings.mock && !testApiKey && !configured) {
      throw new Error('Saisissez une clé API avant de tester.');
    }
    if (testApiKey) {
      await sendWithPromise('saveJemaApiKey', state.settings.provider, testApiKey);
      const check = await sendWithPromise('loadJemaApiKey', state.settings.provider);
      if (!check.apiKey) throw new Error('La clé chiffrée est illisible.');
      if (!state.settings.configuredProviders.includes(state.settings.provider)) {
        state.settings.configuredProviders.push(state.settings.provider);
      }
    }
    const res = await chatCompletion({
      system: 'Tu réponds en un mot.',
      messages: [{ role: 'user', content: 'Dis "ok"' }],
      tools: [],
    });
    ui.toast(`Connexion réussie : « ${(res.text || 'ok').slice(0, 40)} »`, '#00cc66');
    setSettingsStatus(`Connexion réussie avec ${state.settings.provider}.`, 'success');
    saveState();
    ui.updateProviderLabel();
  } catch (e) {
    const message = String(e?.message || e);
    ui.toast(`Échec : ${message.slice(0, 120)}`, '#ef4444');
    setSettingsStatus(`Échec : ${message}`, 'error');
  } finally {
    const configuredProviders = [...state.settings.configuredProviders];
    Object.assign(state.settings, backup, {configuredProviders});
    btn.disabled = false;
    btn.textContent = 'Tester la connexion';
  }
});

// Les dialogs sont fixes : fermeture uniquement par croix ou Annuler.
