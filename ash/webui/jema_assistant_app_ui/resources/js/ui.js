// ===== Rendu UI : barre haute, desks, panneau orchestrateur =====

import { state, getAgent, deleteAgent, publishAgentState } from './state.js';
import { TOOL_DEFS } from './tools.js';

const $ = sel => document.querySelector(sel);

export function esc(s) {
  return String(s ?? '').replace(/[&<>"']/g, c => ({
    '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;',
  }[c]));
}

// ===== Toasts =====

export function toast(text, color) {
  const el = document.createElement('div');
  el.className = 'toast';
  el.style.setProperty('--toast-color', color || 'var(--accent)');
  el.textContent = text;
  $('#toasts').appendChild(el);
  setTimeout(() => {
    el.classList.add('out');
    setTimeout(() => el.remove(), 350);
  }, 3500);
}

// ===== Barre haute =====

export function renderTopBar() {
  const list = $('#agents-list');
  list.innerHTML = '';

  for (const agent of state.agents) {
    const btn = document.createElement('button');
    btn.className = `agent-avatar ${agent.status}${state.activeView === agent.id ? ' active' : ''}`;
    btn.style.setProperty('--agent-color', agent.color);
    btn.title = `${agent.name} — ${agent.role}\n${statusLabel(agent)} (clic : voir le desk)`;

    const r = 21;
    const circ = 2 * Math.PI * r;
    const offset = circ * (1 - (agent.progress || 0) / 100);

    btn.innerHTML = `
      <span class="agent-letter">${esc(agent.letter)}</span>
      <svg viewBox="0 0 50 50">
        <circle class="ring-bg" cx="25" cy="25" r="${r}"/>
        <circle class="ring-fg" cx="25" cy="25" r="${r}"
          stroke-dasharray="${circ}" stroke-dashoffset="${offset}"/>
      </svg>
      ${agent.status === 'done' ? '<span class="badge done">✓</span>' : ''}
      ${agent.status === 'waiting' ? '<span class="badge wait">!</span>' : ''}
    `;
    btn.addEventListener('click', () => {
      // Clic : basculer vers le desk de l'agent pour le voir travailler
      if (agent.status === 'done') agent.status = 'idle';
      switchView(agent.id);
    });
    list.appendChild(btn);
  }
}

function statusLabel(agent) {
  switch (agent.status) {
    case 'working': return `Travaille… ${agent.progress}%`;
    case 'done': return 'Tâche terminée';
    case 'waiting': return 'Attend une action';
    default: return 'Disponible';
  }
}

// ===== Workspace =====

// Icônes du bureau utilisateur (apps système simulées du prototype)
const DESKTOP_APPS = [
  { id: 'files', name: 'Fichiers', icon: '🗂️', color: '#3b82f6' },
  { id: 'browser', name: 'Navigateur', icon: '🌐', color: '#06b6d4' },
  { id: 'jemanote', name: 'JemaNote', icon: '🗒️', color: '#8b5cf6' },
  { id: 'jemapdf', name: 'JemaPDF', icon: '📕', color: '#ef4444' },
  { id: 'calc', name: 'Calculatrice', icon: '🧮', color: '#f59e0b' },
  { id: 'settings', name: 'Paramètres', icon: '⚙️', color: '#64748b' },
];

export function switchView(view) {
  state.activeView = view;
  publishAgentState();
  renderWorkspace();
  renderTopBar();
}

export function renderWorkspace() {
  const ws = $('#workspace');
  if (state.activeView === 'user') {
    renderUserEnv(ws);
  } else {
    const agent = getAgent(state.activeView);
    if (agent) renderAgentDesk(ws, agent);
    else { state.activeView = 'user'; renderUserEnv(ws); }
  }
}

function renderUserEnv(ws) {
  ws.innerHTML = `
    <div class="desktop-layout">
      <nav class="desktop-icons">
        ${DESKTOP_APPS.map(app => `
          <button class="desktop-icon" data-app="${app.id}" title="${esc(app.name)}">
            <span class="icon-tile" style="background:${app.color}22;border:1px solid ${app.color}55;color:${app.color}">${app.icon}</span>
            <span class="icon-label">${esc(app.name)}</span>
          </button>
        `).join('')}
      </nav>
      <section class="desks-area">
        <div class="hero">
          <h1>Environnement utilisateur</h1>
          <p>${state.agents.length
            ? 'Vos agents travaillent chacun dans leur desk. Cliquez sur un desk pour les voir à l’œuvre.'
            : 'Créez votre premier agent avec le bouton + de la barre du haut.'}</p>
        </div>
        <div class="desks-grid">
          ${state.agents.map(a => deskCardHtml(a)).join('')}
        </div>
      </section>
    </div>
  `;
  ws.querySelectorAll('.desk-card').forEach(card => {
    card.addEventListener('click', () => switchView(card.dataset.agent));
  });
  ws.querySelectorAll('.desktop-icon').forEach(btn => {
    btn.addEventListener('click', () => {
      const app = DESKTOP_APPS.find(a => a.id === btn.dataset.app);
      toast(`« ${app.name} » — app système simulée dans ce prototype`, app.color);
    });
  });
}

function deskCardHtml(a) {
  const lastItems = a.items.slice(-3).reverse();
  return `
    <div class="desk-card" data-agent="${a.id}" style="--agent-color:${a.color}">
      <div class="desk-head">
        <div class="desk-dot" style="background:${a.color}">${esc(a.letter)}</div>
        <div>
          <div class="desk-name">${esc(a.name)}</div>
          <div class="desk-role">${esc(a.role)}</div>
        </div>
      </div>
      <div class="desk-status ${a.status}" style="--agent-color:${a.color}">${statusLabel(a)}</div>
      <div class="mini-progress"><div style="width:${a.progress || 0}%;background:${a.color}"></div></div>
      <div class="desk-preview">
        ${lastItems.length
          ? lastItems.map(i => `<div>· ${esc(i.title || i.type)}</div>`).join('')
          : '<div>Aucun livrable pour l’instant</div>'}
      </div>
    </div>
  `;
}

function renderAgentDesk(ws, a) {
  ws.innerHTML = `
    <div class="desk-header" style="--agent-color:${a.color}">
      <div class="desk-dot" style="background:${a.color}">${esc(a.letter)}</div>
      <div class="desk-title">
        <h2>${esc(a.name)}</h2>
        <div class="role">${esc(a.role)} — ${statusLabel(a)}</div>
      </div>
      <div class="chips">${a.skills.map(s => `<span class="chip on">${esc(s)}</span>`).join('')}</div>
      <button class="btn-ghost" id="desk-back" style="margin-left:auto">← Environnement utilisateur</button>
      <button class="btn-ghost" id="desk-delete" title="Supprimer cet agent">🗑</button>
    </div>
    <div class="desk-cols">
      <aside class="activity-panel">
        <h3>Activité en direct</h3>
        <div id="activity-list">
          ${a.activity.length
            ? a.activity.map(act => activityHtml(act)).join('')
            : '<div class="activity-item"><span class="a-icon">💤</span><span>En attente d’une tâche…</span></div>'}
        </div>
      </aside>
      <section>
        ${a.items.length
          ? `<div class="items-grid">${a.items.slice().reverse().map(itemHtml).join('')}</div>`
          : '<div class="empty-desk">Desk vide. Confiez une tâche à cet agent via l’orchestrateur (bouton central ✦) et regardez-le travailler ici.</div>'}
      </section>
    </div>
  `;

  $('#desk-back').addEventListener('click', () => switchView('user'));
  $('#desk-delete').addEventListener('click', () => {
    if (confirm(`Supprimer l'agent « ${a.name} » et son desk ?`)) {
      deleteAgent(a.id);
      switchView('user');
      toast(`Agent « ${a.name} » supprimé`, '#ef4444');
    }
  });

  // Auto-scroll de l'activité vers le bas
  const list = $('#activity-list');
  if (list) list.parentElement.scrollTop = list.parentElement.scrollHeight;
}

function activityHtml(act) {
  const time = new Date(act.ts).toLocaleTimeString('fr-FR', { hour: '2-digit', minute: '2-digit', second: '2-digit' });
  return `
    <div class="activity-item">
      <span class="a-icon">${act.icon}</span>
      <span>${esc(act.text)}</span>
      <span class="a-time">${time}</span>
    </div>
  `;
}

function itemHtml(item) {
  switch (item.type) {
    case 'note':
      return `<div class="item-card note"><h4>📝 ${esc(item.title)}</h4><div class="item-body">${esc(item.content)}</div></div>`;
    case 'document':
      return `<div class="item-card document"><h4>📄 ${esc(item.title)}</h4><div class="item-body">${esc(item.content)}</div></div>`;
    case 'summary':
      return `<div class="item-card summary"><h4>✅ ${esc(item.title)}</h4><div class="item-body">${esc(item.content)}</div></div>`;
    case 'calc':
      return `<div class="item-card calc"><h4>🧮 Calcul</h4><div class="item-body">${esc(item.expression)}</div><div class="calc-result">= ${esc(item.result)}</div></div>`;
    case 'search':
      return `<div class="item-card search"><h4>🔍 ${esc(item.title)}</h4><ul>${(item.results || []).map(r => `<li>${esc(r.title)} <em>(${esc(r.source)})</em></li>`).join('')}</ul></div>`;
    case 'spreadsheet':
      return `
        <div class="item-card spreadsheet"><h4>📊 ${esc(item.title)}</h4>
          <table>
            <thead><tr>${(item.columns || []).map(c => `<th>${esc(c)}</th>`).join('')}</tr></thead>
            <tbody>${(item.rows || []).map(row => `<tr>${(row || []).map(cell => `<td>${esc(cell)}</td>`).join('')}</tr>`).join('')}</tbody>
          </table>
        </div>`;
    default:
      return `<div class="item-card"><h4>${esc(item.title || item.type)}</h4></div>`;
  }
}

// ===== Panneau orchestrateur =====

export function renderOrchMessages() {
  const box = $('#orch-messages');
  box.innerHTML = state.orchMessages.map(m =>
    `<div class="orch-msg ${m.role}">${esc(m.text)}</div>`
  ).join('');
  box.scrollTop = box.scrollHeight;
}

export function setOrchestratorThinking(on) {
  const box = $('#orch-messages');
  const existing = $('#orch-thinking');
  if (on && !existing) {
    const el = document.createElement('div');
    el.id = 'orch-thinking';
    el.className = 'orch-msg thinking';
    el.textContent = 'L’orchestrateur réfléchit…';
    box.appendChild(el);
    box.scrollTop = box.scrollHeight;
  } else if (!on && existing) {
    existing.remove();
  }
}

export function updateProviderLabel() {
  $('#orch-provider-label').textContent = state.settings.mock
    ? 'mode simulation'
    : state.settings.configuredProviders.includes(state.settings.provider)
      ? `${state.settings.provider} · ${state.settings.model || 'défaut'}`
      : 'aucune clé API';
}

// ===== Modals =====

export function openModal(id) { $(id).classList.remove('hidden'); }
export function closeModal(id) { $(id).classList.add('hidden'); }

export function renderToolCheckboxes(selected) {
  const box = $('#agent-tools-checks');
  box.innerHTML = '';
  for (const [name, def] of Object.entries(TOOL_DEFS)) {
    const chip = document.createElement('span');
    chip.className = `chip${selected.includes(name) ? ' on' : ''}`;
    chip.textContent = def.label;
    chip.dataset.tool = name;
    chip.addEventListener('click', () => chip.classList.toggle('on'));
    box.appendChild(chip);
  }
}

export function getCheckedTools() {
  return [...document.querySelectorAll('#agent-tools-checks .chip.on')].map(c => c.dataset.tool);
}
