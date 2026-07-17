// ===== Outils "OS" exécutés réellement dans l'espace de l'agent =====

import { addItem, addActivity } from './state.js';
import {sendWithPromise} from 'chrome://resources/ash/common/cr.m.js';

// Schémas au format OpenAI tools (convertis pour Anthropic dans llm.js)
export const TOOL_DEFS = {
  create_note: {
    icon: '📝',
    label: 'Notes',
    schema: {
      name: 'create_note',
      description: "Créer une note dans l'espace de travail de l'agent",
      parameters: {
        type: 'object',
        properties: {
          title: { type: 'string', description: 'Titre de la note' },
          content: { type: 'string', description: 'Contenu de la note' },
        },
        required: ['title', 'content'],
      },
    },
  },
  create_document: {
    icon: '📄',
    label: 'Documents',
    schema: {
      name: 'create_document',
      description: "Créer un document structuré (rapport, email, spécification…)",
      parameters: {
        type: 'object',
        properties: {
          title: { type: 'string' },
          content: { type: 'string', description: 'Contenu markdown du document' },
        },
        required: ['title', 'content'],
      },
    },
  },
  create_spreadsheet: {
    icon: '📊',
    label: 'Tableaux',
    schema: {
      name: 'create_spreadsheet',
      description: "Créer un tableau de données (colonnes + lignes)",
      parameters: {
        type: 'object',
        properties: {
          title: { type: 'string' },
          columns: { type: 'array', items: { type: 'string' } },
          rows: { type: 'array', items: { type: 'array' } },
        },
        required: ['title', 'columns', 'rows'],
      },
    },
  },
  calculate: {
    icon: '🧮',
    label: 'Calculs',
    schema: {
      name: 'calculate',
      description: "Effectuer un calcul mathématique exact. TOUJOURS utiliser cet outil pour calculer, jamais de tête.",
      parameters: {
        type: 'object',
        properties: {
          expression: { type: 'string', description: "Ex. (245.67 * 8.94) + (156.78 / 3.2)" },
        },
        required: ['expression'],
      },
    },
  },
  web_search: {
    icon: '🔍',
    label: 'Recherche web',
    schema: {
      name: 'web_search',
      description: "Rechercher une information (résultats simulés dans ce prototype)",
      parameters: {
        type: 'object',
        properties: { query: { type: 'string' } },
        required: ['query'],
      },
    },
  },
  notify_user: {
    icon: '🔔',
    label: 'Notifications',
    schema: {
      name: 'notify_user',
      description: "Envoyer une notification à l'utilisateur",
      parameters: {
        type: 'object',
        properties: { message: { type: 'string' } },
        required: ['message'],
      },
    },
  },
  open_app: {
    icon: '🚀',
    label: 'Lancer une application',
    schema: {
      name: 'open_app',
      description: "Lancer une application installée dans JemaOS par son nom",
      parameters: {
        type: 'object',
        properties: { app: { type: 'string' } },
        required: ['app'],
      },
    },
  },
  quicktext_open_content: {
    icon: '✍️',
    label: 'Écrire dans QuickText',
    schema: {
      name: 'quicktext_open_content',
      description: "Ouvrir QuickText avec un texte ou document généré",
      parameters: {
        type: 'object',
        properties: {
          title: { type: 'string' },
          content: { type: 'string' },
        },
        required: ['content'],
      },
    },
  },
  change_language: {
    icon: '🌐',
    label: 'Changer la langue',
    schema: {
      name: 'change_language',
      description: "Changer la langue de l'interface JemaOS. Une déconnexion sera nécessaire.",
      parameters: {
        type: 'object',
        properties: { locale: { type: 'string', description: 'Ex. fr, en-US, de' } },
        required: ['locale'],
      },
    },
  },
  list_display_modes: {
    icon: '🖥️',
    label: "Lister les résolutions",
    schema: {
      name: 'list_display_modes',
      description: "Lister les écrans et leurs modes de résolution valides avant un changement",
      parameters: { type: 'object', properties: {} },
    },
  },
  set_display_mode: {
    icon: '🖥️',
    label: "Changer la résolution",
    schema: {
      name: 'set_display_mode',
      description: "Changer la résolution avec un displayId et modeIndex obtenus par list_display_modes",
      parameters: {
        type: 'object',
        properties: {
          displayId: { type: 'string' },
          modeIndex: { type: 'integer' },
          description: { type: 'string', description: 'Description lisible du mode sélectionné' },
        },
        required: ['displayId', 'modeIndex'],
      },
    },
  },
  search_files: {
    icon: '🔎', label: 'Rechercher des fichiers',
    schema: {name: 'search_files', description: 'Rechercher de vrais fichiers dans MyFiles ou Downloads', parameters: {type: 'object', properties: {root: {type: 'string', enum: ['my_files', 'downloads']}, query: {type: 'string'}}, required: ['query']}},
  },
  read_file: {
    icon: '📖', label: 'Lire un fichier',
    schema: {name: 'read_file', description: 'Lire un fichier texte réel après confirmation', parameters: {type: 'object', properties: {root: {type: 'string', enum: ['my_files', 'downloads']}, path: {type: 'string'}}, required: ['root', 'path']}},
  },
  write_file: {
    icon: '💾', label: 'Créer ou modifier un fichier',
    schema: {name: 'write_file', description: 'Créer TXT, CSV, Markdown ou autre fichier texte réel', parameters: {type: 'object', properties: {root: {type: 'string', enum: ['my_files', 'downloads']}, path: {type: 'string'}, content: {type: 'string'}}, required: ['root', 'path', 'content']}},
  },
  get_audio: {
    icon: '🔊', label: 'État audio',
    schema: {name: 'get_audio', description: 'Lire le volume et le mute', parameters: {type: 'object', properties: {}}},
  },
  set_audio: {
    icon: '🔊', label: 'Régler le son',
    schema: {name: 'set_audio', description: 'Changer le volume ou le mute', parameters: {type: 'object', properties: {volume: {type: 'integer'}, muted: {type: 'boolean'}}}},
  },
  get_power: {
    icon: '🔋', label: 'État batterie',
    schema: {name: 'get_power', description: 'Lire batterie et alimentation', parameters: {type: 'object', properties: {}}},
  },
  native_notification: {
    icon: '🔔', label: 'Notification système',
    schema: {name: 'native_notification', description: 'Afficher une notification native JemaOS', parameters: {type: 'object', properties: {title: {type: 'string'}, message: {type: 'string'}}, required: ['title', 'message']}},
  },
  compose_email: {
    icon: '✉️', label: 'Composer un email',
    schema: {name: 'compose_email', description: 'Ouvrir le client email avec destinataire, sujet et corps préremplis', parameters: {type: 'object', properties: {to: {type: 'string'}, subject: {type: 'string'}, body: {type: 'string'}}, required: ['to']}},
  },
  get_wifi: {icon: '📶', label: 'État Wi-Fi', schema: {name: 'get_wifi', description: 'Lire état Wi-Fi et réseaux visibles', parameters: {type: 'object', properties: {}}}},
  set_wifi: {icon: '📶', label: 'Activer/désactiver Wi-Fi', schema: {name: 'set_wifi', description: 'Activer ou désactiver le Wi-Fi', parameters: {type: 'object', properties: {enabled: {type: 'boolean'}}, required: ['enabled']}}},
  get_bluetooth: {icon: '🅱️', label: 'État Bluetooth', schema: {name: 'get_bluetooth', description: 'Lire état Bluetooth et appareils connus', parameters: {type: 'object', properties: {}}}},
  set_bluetooth: {icon: '🅱️', label: 'Activer/désactiver Bluetooth', schema: {name: 'set_bluetooth', description: 'Activer ou désactiver Bluetooth', parameters: {type: 'object', properties: {enabled: {type: 'boolean'}}, required: ['enabled']}}},
  get_active_window: {icon: '', label: 'Identifier la fenêtre active', schema: {name: 'get_active_window', description: "Identifier l'application, le titre et le Desk de la fenêtre active", parameters: {type: 'object', properties: {}}}},
  open_url: {icon: '', label: 'Ouvrir un site web', schema: {name: 'open_url', description: 'Ouvrir une URL HTTP ou HTTPS dans le navigateur', parameters: {type: 'object', properties: {url: {type: 'string'}}, required: ['url']}}},
  capture_active_window: {icon: '', label: 'Analyser visuellement la fenêtre', schema: {name: 'capture_active_window', description: "Capturer la fenêtre utilisateur active pour une analyse visuelle multimodale", parameters: {type: 'object', properties: {}}}},
  get_accessibility_tree: {icon: '', label: "Lire l'interface accessible", schema: {name: 'get_accessibility_tree', description: "Lire les contrôles sémantiques de la PWA ou page active et obtenir des tokens d'action", parameters: {type: 'object', properties: {}}}},
  accessibility_action: {icon: '', label: "Agir sur un contrôle accessible", schema: {name: 'accessibility_action', description: "Focaliser, activer ou remplir un contrôle avec un token retourné par get_accessibility_tree", parameters: {type: 'object', properties: {token: {type: 'string'}, action: {type: 'string', enum: ['focus', 'activate', 'set_value']}, value: {type: 'string'}}, required: ['token', 'action']}}},
};

// Outils internes toujours disponibles (non décochables)
export const INTERNAL_TOOLS = {
  report_progress: {
    icon: '⏳',
    schema: {
      name: 'report_progress',
      description: "Mettre à jour la progression de la tâche (anneau autour de l'avatar)",
      parameters: {
        type: 'object',
        properties: {
          percent: { type: 'number', description: '0-100' },
          step: { type: 'string', description: 'Étape en cours' },
        },
        required: ['percent'],
      },
    },
  },
  task_complete: {
    icon: '✅',
    schema: {
      name: 'task_complete',
      description: "Clôturer la tâche avec un résumé des livrables produits",
      parameters: {
        type: 'object',
        properties: { summary: { type: 'string' } },
        required: ['summary'],
      },
    },
  },
};

export function schemasFor(agentTools) {
  const all = [...agentTools, 'report_progress', 'task_complete'];
  return all
    .map(name => (TOOL_DEFS[name] || INTERNAL_TOOLS[name])?.schema)
    .filter(Boolean)
    .map(schema => ({ type: 'function', function: schema }));
}

export function toolLabel(name) {
  return (TOOL_DEFS[name] || INTERNAL_TOOLS[name])?.label || name;
}

export function toolIcon(name) {
  return (TOOL_DEFS[name] || INTERNAL_TOOLS[name])?.icon || '🔧';
}


export async function executeSystemTool(agent, request) {
  return await sendWithPromise('executeSystemTool', {
    ...request,
    agentId: agent.id,
  });
}

function authorize(agent, message) {
  if (agent.executionMode === 'blocked') return false;
  if (agent.executionMode === 'autonomous') return true;
  return window.confirm(message);
}

// Exécute l'outil et produit un artefact visible dans le desk de l'agent
export async function executeTool(agent, name, args) {
  switch (name) {
    case 'create_note':
      addItem(agent.id, { type: 'note', title: args.title, content: args.content });
      return { ok: true, message: `Note « ${args.title} » créée` };

    case 'create_document':
      addItem(agent.id, { type: 'document', title: args.title, content: args.content });
      return { ok: true, message: `Document « ${args.title} » créé` };

    case 'create_spreadsheet':
      addItem(agent.id, { type: 'spreadsheet', title: args.title, columns: args.columns || [], rows: args.rows || [] });
      return { ok: true, message: `Tableau « ${args.title} » créé (${(args.rows || []).length} lignes)` };

    case 'calculate': {
      const result = safeEval(args.expression);
      addItem(agent.id, { type: 'calc', title: args.expression, expression: args.expression, result });
      return { ok: true, result };
    }

    case 'web_search': {
      const results = fakeSearchResults(args.query);
      addItem(agent.id, { type: 'search', title: `Recherche : ${args.query}`, results });
      return { ok: true, results };
    }

    case 'notify_user':
      return { ok: true, notify: args.message };

    case 'open_app':
      if (!authorize(agent, `Autoriser l'agent à lancer « ${args.app} » ?`)) {
        return { ok: false, cancelled: true };
      }
      return await executeSystemTool(agent, {
        tool: 'apps.launch', app: args.app, confirmed: true,
      });

    case 'quicktext_open_content':
      if (!authorize(agent,
              `Autoriser l'agent à ouvrir QuickText avec « ${args.title || 'ce document'} » ?`)) {
        return { ok: false, cancelled: true };
      }
      return await executeSystemTool(agent, {
        tool: 'quicktext.open_content',
        title: args.title || 'JemaOS AI',
        content: args.content,
        confirmed: true,
      });

    case 'change_language':
      if (!authorize(agent,
              `Changer la langue de JemaOS en « ${args.locale} » ? Une déconnexion sera nécessaire.`)) {
        return { ok: false, cancelled: true };
      }
      return await executeSystemTool(agent, {
        tool: 'locale.change', locale: args.locale, confirmed: true,
      });

    case 'list_display_modes':
      return await executeSystemTool(agent, {
        tool: 'display.list_modes',
      });

    case 'set_display_mode':
      if (!authorize(agent,
              `Changer la résolution vers « ${args.description || `mode ${args.modeIndex}`} » ?`)) {
        return { ok: false, cancelled: true };
      }
      return await executeSystemTool(agent, {
        tool: 'display.set_mode',
        displayId: args.displayId,
        modeIndex: args.modeIndex,
        confirmed: true,
      });

    case 'search_files':
      return await executeSystemTool(agent, {tool: 'files.search', root: args.root || 'my_files', query: args.query});
    case 'read_file':
      if (!authorize(agent, `Autoriser la lecture du fichier « ${args.path} » ?`)) return {ok: false, cancelled: true};
      return await executeSystemTool(agent, {tool: 'files.read', root: args.root, path: args.path, confirmed: true});
    case 'write_file':
      if (!authorize(agent, `Autoriser l'écriture du fichier « ${args.path} » ?`)) return {ok: false, cancelled: true};
      return await executeSystemTool(agent, {tool: 'files.write', root: args.root, path: args.path, content: args.content, confirmed: true});
    case 'get_audio':
      return await executeSystemTool(agent, {tool: 'audio.get'});
    case 'set_audio':
      if (!authorize(agent, 'Autoriser la modification du volume système ?')) return {ok: false, cancelled: true};
      return await executeSystemTool(agent, {tool: 'audio.set', volume: args.volume, muted: args.muted, confirmed: true});
    case 'get_power':
      return await executeSystemTool(agent, {tool: 'power.get'});
    case 'native_notification':
      return await executeSystemTool(agent, {tool: 'notifications.show', title: args.title, message: args.message});
    case 'compose_email':
      if (!authorize(agent, `Ouvrir un email destiné à « ${args.to} » ?`)) return {ok: false, cancelled: true};
      return await executeSystemTool(agent, {tool: 'email.compose', to: args.to, subject: args.subject || '', body: args.body || '', confirmed: true});
    case 'get_wifi':
      return await executeSystemTool(agent, {tool: 'wifi.get'});
    case 'set_wifi':
      if (!authorize(agent, `${args.enabled ? 'Activer' : 'Désactiver'} le Wi-Fi ?`)) return {ok: false, cancelled: true};
      return await executeSystemTool(agent, {tool: 'wifi.set_enabled', enabled: args.enabled, confirmed: true});
    case 'get_bluetooth':
      return await executeSystemTool(agent, {tool: 'bluetooth.get'});
    case 'set_bluetooth':
      if (!authorize(agent, `${args.enabled ? 'Activer' : 'Désactiver'} le Bluetooth ?`)) return {ok: false, cancelled: true};
      return await executeSystemTool(agent, {tool: 'bluetooth.set_enabled', enabled: args.enabled, confirmed: true});
    case 'get_active_window':
      return await executeSystemTool(agent, {tool: 'environment.get_active_window'});
    case 'open_url':
      if (!authorize(agent, `Ouvrir « ${args.url} » dans le navigateur ?`)) return {ok: false, cancelled: true};
      return await executeSystemTool(agent, {tool: 'browser.open_url', url: args.url, confirmed: true});
    case 'capture_active_window':
      if (!authorize(agent, 'Autoriser l’analyse visuelle de la fenêtre active ?')) return {ok: false, cancelled: true};
      return await executeSystemTool(agent, {tool: 'environment.capture_active_window', confirmed: true});
    case 'get_accessibility_tree':
      if (!authorize(agent, "Autoriser la lecture de l'interface accessible de la fenêtre active ?")) return {ok: false, cancelled: true};
      return await executeSystemTool(agent, {tool: 'accessibility.get_active_tree', confirmed: true});
    case 'accessibility_action':
      if (!authorize(agent, `Autoriser l'action « ${args.action} » sur l'interface ?`)) return {ok: false, cancelled: true};
      {
        const dispatched = await executeSystemTool(agent, {tool: 'accessibility.perform_action', token: args.token, action: args.action, value: args.value || '', confirmed: true});
        await new Promise(resolve => setTimeout(resolve, 450));
        try {
          const afterState = await executeSystemTool(agent, {tool: 'accessibility.get_active_tree', confirmed: true});
          return {dispatched, afterState, verifiedByObservation: true};
        } catch (error) {
          return {dispatched, verifiedByObservation: false, verificationError: String(error)};
        }
      }

    case 'report_progress':
      return { ok: true, percent: args.percent, step: args.step };

    case 'task_complete':
      addItem(agent.id, { type: 'summary', title: 'Rapport de tâche', content: args.summary });
      return { ok: true, done: true, summary: args.summary };

    default:
      return { ok: false, error: `Outil inconnu: ${name}` };
  }
}

function safeEval(expr) {
  if (typeof expr !== 'string' || !/^[\d\s+\-*/().,%]*$/.test(expr)) {
    throw new Error('Expression invalide (chiffres et + - * / % ( ) . uniquement)');
  }
  const cleaned = expr.replace(/,/g, '.').replace(/%/g, '/100');
  const r = Function(`"use strict"; return (${cleaned});`)();
  if (typeof r !== 'number' || !Number.isFinite(r)) throw new Error('Résultat invalide');
  return Math.round(r * 10000) / 10000;
}

function fakeSearchResults(query) {
  return [
    { title: `${query} — vue d'ensemble`, source: 'qwant.com' },
    { title: `${query} : guide complet 2026`, source: 'documentation' },
    { title: `Actualités récentes sur ${query}`, source: 'presse' },
  ];
}
