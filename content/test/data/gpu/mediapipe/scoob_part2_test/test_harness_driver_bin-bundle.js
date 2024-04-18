//javascript/closure/base.js
/**
 * @license
 * Copyright The Closure Library Authors.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @fileoverview Bootstrap for the Google JS Library (Closure).
 *
 * In uncompiled mode base.js will attempt to load Closure's deps file, unless
 * the global <code>CLOSURE_NO_DEPS</code> is set to true.  This allows projects
 * to include their own deps file(s) from different locations.
 *
 * Avoid including base.js more than once. This is strictly discouraged and not
 * supported. goog.require(...) won't work properly in that case.
 *
 * @provideGoog
 */


/**
 * @define {boolean} Overridden to true by the compiler.
 */
var COMPILED = false;


/**
 * Base namespace for the Closure library.  Checks to see goog is already
 * defined in the current scope before assigning to prevent clobbering if
 * base.js is loaded more than once.
 *
 * @const
 */
var goog = goog || {};

/**
 * Reference to the global object.
 * https://www.ecma-international.org/ecma-262/9.0/index.html#sec-global-object
 *
 * More info on this implementation here:
 * https://docs.google.com/document/d/1NAeW4Wk7I7FV0Y2tcUFvQdGMc89k2vdgSXInw8_nvCI/edit
 *
 * @const
 * @suppress {undefinedVars} self won't be referenced unless `this` is falsy.
 * @type {!Global}
 */
goog.global =
    // Check `this` first for backwards compatibility.
    // Valid unless running as an ES module or in a function wrapper called
    //   without setting `this` properly.
    // Note that base.js can't usefully be imported as an ES module, but it may
    // be compiled into bundles that are loadable as ES modules.
    this ||
    // https://developer.mozilla.org/en-US/docs/Web/API/Window/self
    // For in-page browser environments and workers.
    self;


/**
 * A hook for overriding the define values in uncompiled mode.
 *
 * In uncompiled mode, `CLOSURE_UNCOMPILED_DEFINES` may be defined before
 * loading base.js.  If a key is defined in `CLOSURE_UNCOMPILED_DEFINES`,
 * `goog.define` will use the value instead of the default value.  This
 * allows flags to be overwritten without compilation (this is normally
 * accomplished with the compiler's "define" flag).
 *
 * Example:
 * <pre>
 *   var CLOSURE_UNCOMPILED_DEFINES = {'goog.DEBUG': false};
 * </pre>
 *
 * @type {Object<string, (string|number|boolean)>|undefined}
 */
goog.global.CLOSURE_UNCOMPILED_DEFINES;


/**
 * A hook for overriding the define values in uncompiled or compiled mode,
 * like CLOSURE_UNCOMPILED_DEFINES but effective in compiled code.  In
 * uncompiled code CLOSURE_UNCOMPILED_DEFINES takes precedence.
 *
 * Also unlike CLOSURE_UNCOMPILED_DEFINES the values must be number, boolean or
 * string literals or the compiler will emit an error.
 *
 * While any @define value may be set, only those set with goog.define will be
 * effective for uncompiled code.
 *
 * Example:
 * <pre>
 *   var CLOSURE_DEFINES = {'goog.DEBUG': false} ;
 * </pre>
 *
 * @type {Object<string, (string|number|boolean)>|undefined}
 */
goog.global.CLOSURE_DEFINES;


/**
 * Builds an object structure for the provided namespace path, ensuring that
 * names that already exist are not overwritten. For example:
 * "a.b.c" -> a = {};a.b={};a.b.c={};
 * Used by goog.provide and goog.exportSymbol.
 * @param {string} name The name of the object that this file defines.
 * @param {*=} object The object to expose at the end of the path.
 * @param {boolean=} overwriteImplicit If object is set and a previous call
 *     implicitly constructed the namespace given by name, this parameter
 *     controls whether object should overwrite the implicitly constructed
 *     namespace or be merged into it. Defaults to false.
 * @param {?Object=} objectToExportTo The object to add the path to; if this
 *     field is not specified, its value defaults to `goog.global`.
 * @private
 */
goog.exportPath_ = function(name, object, overwriteImplicit, objectToExportTo) {
  var parts = name.split('.');
  var cur = objectToExportTo || goog.global;

  // Internet Explorer exhibits strange behavior when throwing errors from
  // methods externed in this manner.  See the testExportSymbolExceptions in
  // base_test.html for an example.
  if (!(parts[0] in cur) && typeof cur.execScript != 'undefined') {
    cur.execScript('var ' + parts[0]);
  }

  for (var part; parts.length && (part = parts.shift());) {
    if (!parts.length && object !== undefined) {
      if (!overwriteImplicit && goog.isObject(object) &&
          goog.isObject(cur[part])) {
        // Merge properties on object (the input parameter) with the existing
        // implicitly defined namespace, so as to not clobber previously
        // defined child namespaces.
        for (var prop in object) {
          if (object.hasOwnProperty(prop)) {
            cur[part][prop] = object[prop];
          }
        }
      } else {
        // Either there is no existing implicit namespace, or overwriteImplicit
        // is set to true, so directly assign object (the input parameter) to
        // the namespace.
        cur[part] = object;
      }
    } else if (cur[part] && cur[part] !== Object.prototype[part]) {
      cur = cur[part];
    } else {
      cur = cur[part] = {};
    }
  }
};


/**
 * Defines a named value. In uncompiled mode, the value is retrieved from
 * CLOSURE_DEFINES or CLOSURE_UNCOMPILED_DEFINES if the object is defined and
 * has the property specified, and otherwise used the defined defaultValue.
 * When compiled the default can be overridden using the compiler options or the
 * value set in the CLOSURE_DEFINES object. Returns the defined value so that it
 * can be used safely in modules. Note that the value type MUST be either
 * boolean, number, or string.
 *
 * @param {string} name The distinguished name to provide.
 * @param {T} defaultValue
 * @return {T} The defined value.
 * @template T
 */
goog.define = function(name, defaultValue) {
  var value = defaultValue;
  if (!COMPILED) {
    var uncompiledDefines = goog.global.CLOSURE_UNCOMPILED_DEFINES;
    var defines = goog.global.CLOSURE_DEFINES;
    if (uncompiledDefines &&
        // Anti DOM-clobbering runtime check (b/37736576).
        /** @type {?} */ (uncompiledDefines).nodeType === undefined &&
        Object.prototype.hasOwnProperty.call(uncompiledDefines, name)) {
      value = uncompiledDefines[name];
    } else if (
        defines &&
        // Anti DOM-clobbering runtime check (b/37736576).
        /** @type {?} */ (defines).nodeType === undefined &&
        Object.prototype.hasOwnProperty.call(defines, name)) {
      value = defines[name];
    }
  }
  return value;
};


/**
 * @define {number} Integer year indicating the set of browser features that are
 * guaranteed to be present.  This is defined to include exactly features that
 * work correctly on all "modern" browsers that are stable on January 1 of the
 * specified year.  For example,
 * ```js
 * if (goog.FEATURESET_YEAR >= 2019) {
 *   // use APIs known to be available on all major stable browsers Jan 1, 2019
 * } else {
 *   // polyfill for older browsers
 * }
 * ```
 * This is intended to be the primary define for removing
 * unnecessary browser compatibility code (such as ponyfills and workarounds),
 * and should inform the default value for most other defines:
 * ```js
 * const ASSUME_NATIVE_PROMISE =
 *     goog.define('ASSUME_NATIVE_PROMISE', goog.FEATURESET_YEAR >= 2016);
 * ```
 *
 * The default assumption is that IE9 is the lowest supported browser, which was
 * first available Jan 1, 2012.
 *
 * TODO(mathiasb): Reference more thorough documentation when it's available.
 */
goog.FEATURESET_YEAR = goog.define('goog.FEATURESET_YEAR', 2012);


/**
 * @define {boolean} DEBUG is provided as a convenience so that debugging code
 * that should not be included in a production. It can be easily stripped
 * by specifying --define goog.DEBUG=false to the Closure Compiler aka
 * JSCompiler. For example, most toString() methods should be declared inside an
 * "if (goog.DEBUG)" conditional because they are generally used for debugging
 * purposes and it is difficult for the JSCompiler to statically determine
 * whether they are used.
 */
goog.DEBUG = goog.define('goog.DEBUG', true);


/**
 * @define {string} LOCALE defines the locale being used for compilation. It is
 * used to select locale specific data to be compiled in js binary. BUILD rule
 * can specify this value by "--define goog.LOCALE=<locale_name>" as a compiler
 * option.
 *
 * Take into account that the locale code format is important. You should use
 * the canonical Unicode format with hyphen as a delimiter. Language must be
 * lowercase, Language Script - Capitalized, Region - UPPERCASE.
 * There are few examples: pt-BR, en, en-US, sr-Latin-BO, zh-Hans-CN.
 *
 * See more info about locale codes here:
 * http://www.unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
 *
 * For language codes you should use values defined by ISO 693-1. See it here
 * http://www.w3.org/WAI/ER/IG/ert/iso639.htm. There is only one exception from
 * this rule: the Hebrew language. For legacy reasons the old code (iw) should
 * be used instead of the new code (he).
 *
 * MOE:begin_intracomment_strip
 * See http://g3doc/i18n/identifiers/g3doc/synonyms.
 * MOE:end_intracomment_strip
 */
goog.LOCALE = goog.define('goog.LOCALE', 'en');  // default to en


/**
 * @define {boolean} Whether this code is running on trusted sites.
 *
 * On untrusted sites, several native functions can be defined or overridden by
 * external libraries like Prototype, Datejs, and JQuery and setting this flag
 * to false forces closure to use its own implementations when possible.
 *
 * If your JavaScript can be loaded by a third party site and you are wary about
 * relying on non-standard implementations, specify
 * "--define goog.TRUSTED_SITE=false" to the compiler.
 */
goog.TRUSTED_SITE = goog.define('goog.TRUSTED_SITE', true);


/**
 * @define {boolean} Whether code that calls {@link goog.setTestOnly} should
 *     be disallowed in the compilation unit.
 */
goog.DISALLOW_TEST_ONLY_CODE =
    goog.define('goog.DISALLOW_TEST_ONLY_CODE', COMPILED && !goog.DEBUG);


/**
 * @define {boolean} Whether to use a Chrome app CSP-compliant method for
 *     loading scripts via goog.require. @see appendScriptSrcNode_.
 */
goog.ENABLE_CHROME_APP_SAFE_SCRIPT_LOADING =
    goog.define('goog.ENABLE_CHROME_APP_SAFE_SCRIPT_LOADING', false);


/**
 * Defines a namespace in Closure.
 *
 * A namespace may only be defined once in a codebase. It may be defined using
 * goog.provide() or goog.module().
 *
 * The presence of one or more goog.provide() calls in a file indicates
 * that the file defines the given objects/namespaces.
 * Provided symbols must not be null or undefined.
 *
 * In addition, goog.provide() creates the object stubs for a namespace
 * (for example, goog.provide("goog.foo.bar") will create the object
 * goog.foo.bar if it does not already exist).
 *
 * Build tools also scan for provide/require/module statements
 * to discern dependencies, build dependency files (see deps.js), etc.
 *
 * @see goog.require
 * @see goog.module
 * @param {string} name Namespace provided by this file in the form
 *     "goog.package.part".
 * deprecated Use goog.module (see b/159289405)
 */
goog.provide = function(name) {
  if (goog.isInModuleLoader_()) {
    throw new Error('goog.provide cannot be used within a module.');
  }
  if (!COMPILED) {
    // Ensure that the same namespace isn't provided twice.
    // A goog.module/goog.provide maps a goog.require to a specific file
    if (goog.isProvided_(name)) {
      throw new Error('Namespace "' + name + '" already declared.');
    }
  }

  goog.constructNamespace_(name);
};


/**
 * @param {string} name Namespace provided by this file in the form
 *     "goog.package.part".
 * @param {?Object=} object The object to embed in the namespace.
 * @param {boolean=} overwriteImplicit If object is set and a previous call
 *     implicitly constructed the namespace given by name, this parameter
 *     controls whether opt_obj should overwrite the implicitly constructed
 *     namespace or be merged into it. Defaults to false.
 * @private
 */
goog.constructNamespace_ = function(name, object, overwriteImplicit) {
  if (!COMPILED) {
    delete goog.implicitNamespaces_[name];

    var namespace = name;
    while ((namespace = namespace.substring(0, namespace.lastIndexOf('.')))) {
      if (goog.getObjectByName(namespace)) {
        break;
      }
      goog.implicitNamespaces_[namespace] = true;
    }
  }

  goog.exportPath_(name, object, overwriteImplicit);
};


/**
 * Returns CSP nonce, if set for any script tag.
 * @param {?Window=} opt_window The window context used to retrieve the nonce.
 *     Defaults to global context.
 * @return {string} CSP nonce or empty string if no nonce is present.
 * @deprecated Use goog.dom.safe.getScriptNonce.
 */
goog.getScriptNonce = function(opt_window) {
  if (opt_window && opt_window != goog.global) {
    return goog.getScriptNonce_(opt_window.document);
  }
  if (goog.cspNonce_ === null) {
    goog.cspNonce_ = goog.getScriptNonce_(goog.global.document);
  }
  return goog.cspNonce_;
};


/**
 * According to the CSP3 spec a nonce must be a valid base64 string.
 * @see https://www.w3.org/TR/CSP3/#grammardef-base64-value
 * @private @const
 */
goog.NONCE_PATTERN_ = /^[\w+/_-]+[=]{0,2}$/;


/**
 * @private {?string}
 */
goog.cspNonce_ = null;


/**
 * Returns CSP nonce, if set for any script tag.
 * @param {!Document} doc
 * @return {string} CSP nonce or empty string if no nonce is present.
 * @private
 */
goog.getScriptNonce_ = function(doc) {
  var script = doc.querySelector && doc.querySelector('script[nonce]');
  if (script) {
    // Try to get the nonce from the IDL property first, because browsers that
    // implement additional nonce protection features (currently only Chrome) to
    // prevent nonce stealing via CSS do not expose the nonce via attributes.
    // See https://github.com/whatwg/html/issues/2369
    var nonce = script['nonce'] || script.getAttribute('nonce');
    if (nonce && goog.NONCE_PATTERN_.test(nonce)) {
      return nonce;
    }
  }
  return '';
};


/**
 * Module identifier validation regexp.
 * Note: This is a conservative check, it is very possible to be more lenient,
 *   the primary exclusion here is "/" and "\" and a leading ".", these
 *   restrictions are intended to leave the door open for using goog.require
 *   with relative file paths rather than module identifiers.
 * @private
 */
goog.VALID_MODULE_RE_ = /^[a-zA-Z_$][a-zA-Z0-9._$]*$/;


/**
 * Defines a module in Closure.
 *
 * Marks that this file must be loaded as a module and claims the namespace.
 *
 * A namespace may only be defined once in a codebase. It may be defined using
 * goog.provide() or goog.module().
 *
 * goog.module() has three requirements:
 * - goog.module may not be used in the same file as goog.provide.
 * - goog.module must be the first statement in the file.
 * - only one goog.module is allowed per file.
 *
 * When a goog.module annotated file is loaded, it is enclosed in
 * a strict function closure. This means that:
 * - any variables declared in a goog.module file are private to the file
 * (not global), though the compiler is expected to inline the module.
 * - The code must obey all the rules of "strict" JavaScript.
 * - the file will be marked as "use strict"
 *
 * NOTE: unlike goog.provide, goog.module does not declare any symbols by
 * itself. If declared symbols are desired, use
 * goog.module.declareLegacyNamespace().
 *
 * MOE:begin_intracomment_strip
 * See the goog.module announcement at http://go/goog.module-announce
 * MOE:end_intracomment_strip
 *
 * See the public goog.module proposal: http://goo.gl/Va1hin
 *
 * @param {string} name Namespace provided by this file in the form
 *     "goog.package.part", is expected but not required.
 * @return {void}
 */
goog.module = function(name) {
  if (typeof name !== 'string' || !name ||
      name.search(goog.VALID_MODULE_RE_) == -1) {
    throw new Error('Invalid module identifier');
  }
  if (!goog.isInGoogModuleLoader_()) {
    throw new Error(
        'Module ' + name + ' has been loaded incorrectly. Note, ' +
        'modules cannot be loaded as normal scripts. They require some kind of ' +
        'pre-processing step. You\'re likely trying to load a module via a ' +
        'script tag or as a part of a concatenated bundle without rewriting the ' +
        'module. For more info see: ' +
        'https://github.com/google/closure-library/wiki/goog.module:-an-ES6-module-like-alternative-to-goog.provide.');
  }
  if (goog.moduleLoaderState_.moduleName) {
    throw new Error('goog.module may only be called once per module.');
  }

  // Store the module name for the loader.
  goog.moduleLoaderState_.moduleName = name;
  if (!COMPILED) {
    // Ensure that the same namespace isn't provided twice.
    // A goog.module/goog.provide maps a goog.require to a specific file
    if (goog.isProvided_(name)) {
      throw new Error('Namespace "' + name + '" already declared.');
    }
    delete goog.implicitNamespaces_[name];
  }
};


/**
 * @param {string} name The module identifier.
 * @return {?} The module exports for an already loaded module or null.
 *
 * Note: This is not an alternative to goog.require, it does not
 * indicate a hard dependency, instead it is used to indicate
 * an optional dependency or to access the exports of a module
 * that has already been loaded.
 * @suppress {missingProvide}
 */
goog.module.get = function(name) {
  return goog.module.getInternal_(name);
};


/**
 * @param {string} name The module identifier.
 * @return {?} The module exports for an already loaded module or null.
 * @private
 */
goog.module.getInternal_ = function(name) {
  if (!COMPILED) {
    if (name in goog.loadedModules_) {
      return goog.loadedModules_[name].exports;
    } else if (!goog.implicitNamespaces_[name]) {
      var ns = goog.getObjectByName(name);
      return ns != null ? ns : null;
    }
  }
  return null;
};


/**
 * Types of modules the debug loader can load.
 * @enum {string}
 */
goog.ModuleType = {
  ES6: 'es6',
  GOOG: 'goog'
};


/**
 * @private {?{
 *   moduleName: (string|undefined),
 *   declareLegacyNamespace:boolean,
 *   type: ?goog.ModuleType
 * }}
 */
goog.moduleLoaderState_ = null;


/**
 * @private
 * @return {boolean} Whether a goog.module or an es6 module is currently being
 *     initialized.
 */
goog.isInModuleLoader_ = function() {
  return goog.isInGoogModuleLoader_() || goog.isInEs6ModuleLoader_();
};


/**
 * @private
 * @return {boolean} Whether a goog.module is currently being initialized.
 */
goog.isInGoogModuleLoader_ = function() {
  return !!goog.moduleLoaderState_ &&
      goog.moduleLoaderState_.type == goog.ModuleType.GOOG;
};


/**
 * @private
 * @return {boolean} Whether an es6 module is currently being initialized.
 */
goog.isInEs6ModuleLoader_ = function() {
  var inLoader = !!goog.moduleLoaderState_ &&
      goog.moduleLoaderState_.type == goog.ModuleType.ES6;

  if (inLoader) {
    return true;
  }

  var jscomp = goog.global['$jscomp'];

  if (jscomp) {
    // jscomp may not have getCurrentModulePath if this is a compiled bundle
    // that has some of the runtime, but not all of it. This can happen if
    // optimizations are turned on so the unused runtime is removed but renaming
    // and Closure pass are off (so $jscomp is still named $jscomp and the
    // goog.provide/require calls still exist).
    if (typeof jscomp.getCurrentModulePath != 'function') {
      return false;
    }

    // Bundled ES6 module.
    return !!jscomp.getCurrentModulePath();
  }

  return false;
};


/**
 * Provide the module's exports as a globally accessible object under the
 * module's declared name.  This is intended to ease migration to goog.module
 * for files that have existing usages.
 * @suppress {missingProvide}
 */
goog.module.declareLegacyNamespace = function() {
  if (!COMPILED && !goog.isInGoogModuleLoader_()) {
    throw new Error(
        'goog.module.declareLegacyNamespace must be called from ' +
        'within a goog.module');
  }
  if (!COMPILED && !goog.moduleLoaderState_.moduleName) {
    throw new Error(
        'goog.module must be called prior to ' +
        'goog.module.declareLegacyNamespace.');
  }
  goog.moduleLoaderState_.declareLegacyNamespace = true;
};


/**
 * Associates an ES6 module with a Closure module ID so that is available via
 * goog.require. The associated ID  acts like a goog.module ID - it does not
 * create any global names, it is merely available via goog.require /
 * goog.module.get / goog.forwardDeclare / goog.requireType. goog.require and
 * goog.module.get will return the entire module as if it was import *'d. This
 * allows Closure files to reference ES6 modules for the sake of migration.
 *
 * @param {string} namespace
 * @suppress {missingProvide}
 */
goog.declareModuleId = function(namespace) {
  if (!COMPILED) {
    if (!goog.isInEs6ModuleLoader_()) {
      throw new Error(
          'goog.declareModuleId may only be called from ' +
          'within an ES6 module');
    }
    if (goog.moduleLoaderState_ && goog.moduleLoaderState_.moduleName) {
      throw new Error(
          'goog.declareModuleId may only be called once per module.');
    }
    if (namespace in goog.loadedModules_) {
      throw new Error(
          'Module with namespace "' + namespace + '" already exists.');
    }
  }
  if (goog.moduleLoaderState_) {
    // Not bundled - debug loading.
    goog.moduleLoaderState_.moduleName = namespace;
  } else {
    // Bundled - not debug loading, no module loader state.
    var jscomp = goog.global['$jscomp'];
    if (!jscomp || typeof jscomp.getCurrentModulePath != 'function') {
      throw new Error(
          'Module with namespace "' + namespace +
          '" has been loaded incorrectly.');
    }
    var exports = jscomp.require(jscomp.getCurrentModulePath());
    goog.loadedModules_[namespace] = {
      exports: exports,
      type: goog.ModuleType.ES6,
      moduleId: namespace
    };
  }
};


/**
 * Marks that the current file should only be used for testing, and never for
 * live code in production.
 *
 * In the case of unit tests, the message may optionally be an exact namespace
 * for the test (e.g. 'goog.stringTest'). The linter will then ignore the extra
 * provide (if not explicitly defined in the code).
 *
 * @param {string=} opt_message Optional message to add to the error that's
 *     raised when used in production code.
 */
goog.setTestOnly = function(opt_message) {
  if (goog.DISALLOW_TEST_ONLY_CODE) {
    opt_message = opt_message || '';
    throw new Error(
        'Importing test-only code into non-debug environment' +
        (opt_message ? ': ' + opt_message : '.'));
  }
};


/**
 * Forward declares a symbol. This is an indication to the compiler that the
 * symbol may be used in the source yet is not required and may not be provided
 * in compilation.
 *
 * The most common usage of forward declaration is code that takes a type as a
 * function parameter but does not need to require it. By forward declaring
 * instead of requiring, no hard dependency is made, and (if not required
 * elsewhere) the namespace may never be required and thus, not be pulled
 * into the JavaScript binary. If it is required elsewhere, it will be type
 * checked as normal.
 *
 * Before using goog.forwardDeclare, please read the documentation at
 * https://github.com/google/closure-compiler/wiki/Bad-Type-Annotation to
 * understand the options and tradeoffs when working with forward declarations.
 *
 * @param {string} name The namespace to forward declare in the form of
 *     "goog.package.part".
 * @deprecated See go/noforwarddeclaration, Use `goog.requireType` instead.
 */
goog.forwardDeclare = function(name) {};


/**
 * Forward declare type information. Used to assign types to goog.global
 * referenced object that would otherwise result in unknown type references
 * and thus block property disambiguation.
 */
goog.forwardDeclare('Document');
goog.forwardDeclare('HTMLScriptElement');
goog.forwardDeclare('XMLHttpRequest');


if (!COMPILED) {
  /**
   * Check if the given name has been goog.provided. This will return false for
   * names that are available only as implicit namespaces.
   * @param {string} name name of the object to look for.
   * @return {boolean} Whether the name has been provided.
   * @private
   */
  goog.isProvided_ = function(name) {
    return (name in goog.loadedModules_) ||
        (!goog.implicitNamespaces_[name] && goog.getObjectByName(name) != null);
  };

  /**
   * Namespaces implicitly defined by goog.provide. For example,
   * goog.provide('goog.events.Event') implicitly declares that 'goog' and
   * 'goog.events' must be namespaces.
   *
   * @type {!Object<string, (boolean|undefined)>}
   * @private
   */
  goog.implicitNamespaces_ = {'goog.module': true};

  // NOTE: We add goog.module as an implicit namespace as goog.module is defined
  // here and because the existing module package has not been moved yet out of
  // the goog.module namespace. This satisifies both the debug loader and
  // ahead-of-time dependency management.
}


/**
 * Returns an object based on its fully qualified external name.  The object
 * is not found if null or undefined.  If you are using a compilation pass that
 * renames property names beware that using this function will not find renamed
 * properties.
 *
 * @param {string} name The fully qualified name.
 * @param {Object=} opt_obj The object within which to look; default is
 *     |goog.global|.
 * @return {?} The value (object or primitive) or, if not found, null.
 */
goog.getObjectByName = function(name, opt_obj) {
  var parts = name.split('.');
  var cur = opt_obj || goog.global;
  for (var i = 0; i < parts.length; i++) {
    cur = cur[parts[i]];
    if (cur == null) {
      return null;
    }
  }
  return cur;
};


/**
 * Adds a dependency from a file to the files it requires.
 * @param {string} relPath The path to the js file.
 * @param {!Array<string>} provides An array of strings with
 *     the names of the objects this file provides.
 * @param {!Array<string>} requires An array of strings with
 *     the names of the objects this file requires.
 * @param {boolean|!Object<string>=} opt_loadFlags Parameters indicating
 *     how the file must be loaded.  The boolean 'true' is equivalent
 *     to {'module': 'goog'} for backwards-compatibility.  Valid properties
 *     and values include {'module': 'goog'} and {'lang': 'es6'}.
 */
goog.addDependency = function(relPath, provides, requires, opt_loadFlags) {
  if (!COMPILED && goog.DEPENDENCIES_ENABLED) {
    goog.debugLoader_.addDependency(relPath, provides, requires, opt_loadFlags);
  }
};


// MOE:begin_strip
/**
 * Whether goog.require should throw an exception if it fails.
 * @type {boolean}
 */
goog.useStrictRequires = false;


// MOE:end_strip


// NOTE(nnaze): The debug DOM loader was included in base.js as an original way
// to do "debug-mode" development.  The dependency system can sometimes be
// confusing, as can the debug DOM loader's asynchronous nature.
//
// With the DOM loader, a call to goog.require() is not blocking -- the script
// will not load until some point after the current script.  If a namespace is
// needed at runtime, it needs to be defined in a previous script, or loaded via
// require() with its registered dependencies.
//
// User-defined namespaces may need their own deps file. For a reference on
// creating a deps file, see:
// MOE:begin_strip
// Internally: http://go/deps-files and http://go/be#js_deps
// MOE:end_strip
// Externally: https://developers.google.com/closure/library/docs/depswriter
//
// Because of legacy clients, the DOM loader can't be easily removed from
// base.js.  Work was done to make it disableable or replaceable for
// different environments (DOM-less JavaScript interpreters like Rhino or V8,
// for example). See bootstrap/ for more information.


/**
 * @define {boolean} Whether to enable the debug loader.
 *
 * If enabled, a call to goog.require() will attempt to load the namespace by
 * appending a script tag to the DOM (if the namespace has been registered).
 *
 * If disabled, goog.require() will simply assert that the namespace has been
 * provided (and depend on the fact that some outside tool correctly ordered
 * the script).
 */
goog.ENABLE_DEBUG_LOADER = goog.define('goog.ENABLE_DEBUG_LOADER', true);


/**
 * @param {string} msg
 * @private
 */
goog.logToConsole_ = function(msg) {
  if (goog.global.console) {
    goog.global.console['error'](msg);
  }
};


/**
 * Implements a system for the dynamic resolution of dependencies that works in
 * parallel with the BUILD system.
 *
 * Note that all calls to goog.require will be stripped by the compiler.
 *
 * @see goog.provide
 * @param {string} namespace Namespace (as was given in goog.provide,
 *     goog.module, or goog.declareModuleId) in the form
 *     "goog.package.part".
 * @return {?} If called within a goog.module or ES6 module file, the associated
 *     namespace or module otherwise null.
 */
goog.require = function(namespace) {
  if (!COMPILED) {
    // Might need to lazy load on old IE.
    if (goog.ENABLE_DEBUG_LOADER) {
      goog.debugLoader_.requested(namespace);
    }

    // If the object already exists we do not need to do anything.
    if (goog.isProvided_(namespace)) {
      if (goog.isInModuleLoader_()) {
        return goog.module.getInternal_(namespace);
      }
    } else if (goog.ENABLE_DEBUG_LOADER) {
      var moduleLoaderState = goog.moduleLoaderState_;
      goog.moduleLoaderState_ = null;
      try {
        goog.debugLoader_.load_(namespace);
      } finally {
        goog.moduleLoaderState_ = moduleLoaderState;
      }
    }

    return null;
  }
};


/**
 * Requires a symbol for its type information. This is an indication to the
 * compiler that the symbol may appear in type annotations, yet it is not
 * referenced at runtime.
 *
 * When called within a goog.module or ES6 module file, the return value may be
 * assigned to or destructured into a variable, but it may not be otherwise used
 * in code outside of a type annotation.
 *
 * Note that all calls to goog.requireType will be stripped by the compiler.
 *
 * @param {string} namespace Namespace (as was given in goog.provide,
 *     goog.module, or goog.declareModuleId) in the form
 *     "goog.package.part".
 * @return {?}
 */
goog.requireType = function(namespace) {
  // Return an empty object so that single-level destructuring of the return
  // value doesn't crash at runtime when using the debug loader. Multi-level
  // destructuring isn't supported.
  return {};
};


/**
 * Path for included scripts.
 * @type {string}
 */
goog.basePath = '';


/**
 * A hook for overriding the base path.
 * @type {string|undefined}
 */
goog.global.CLOSURE_BASE_PATH;


/**
 * Whether to attempt to load Closure's deps file. By default, when uncompiled,
 * deps files will attempt to be loaded.
 * @type {boolean|undefined}
 */
goog.global.CLOSURE_NO_DEPS;


/**
 * A function to import a single script. This is meant to be overridden when
 * Closure is being run in non-HTML contexts, such as web workers. It's defined
 * in the global scope so that it can be set before base.js is loaded, which
 * allows deps.js to be imported properly.
 *
 * The first parameter the script source, which is a relative URI. The second,
 * optional parameter is the script contents, in the event the script needed
 * transformation. It should return true if the script was imported, false
 * otherwise.
 * @type {(function(string, string=): boolean)|undefined}
 */
goog.global.CLOSURE_IMPORT_SCRIPT;


/**
 * Null function used for default values of callbacks, etc.
 * @return {void} Nothing.
 * @deprecated use '()=>{}' or 'function(){}' instead.
 */
goog.nullFunction = function() {};


/**
 * When defining a class Foo with an abstract method bar(), you can do:
 * Foo.prototype.bar = goog.abstractMethod
 *
 * Now if a subclass of Foo fails to override bar(), an error will be thrown
 * when bar() is invoked.
 *
 * @type {!Function}
 * @throws {Error} when invoked to indicate the method should be overridden.
 * @deprecated Use "@abstract" annotation instead of goog.abstractMethod in new
 *     code. See
 *     https://github.com/google/closure-compiler/wiki/@abstract-classes-and-methods
 */
goog.abstractMethod = function() {
  throw new Error('unimplemented abstract method');
};


/**
 * Adds a `getInstance` static method that always returns the same
 * instance object.
 * @param {!Function} ctor The constructor for the class to add the static
 *     method to.
 * @suppress {missingProperties} 'instance_' isn't a property on 'Function'
 *     but we don't have a better type to use here.
 */
goog.addSingletonGetter = function(ctor) {
  // instance_ is immediately set to prevent issues with sealed constructors
  // such as are encountered when a constructor is returned as the export object
  // of a goog.module in unoptimized code.
  // Delcare type to avoid conformance violations that ctor.instance_ is unknown
  /** @type {undefined|!Object} @suppress {underscore} */
  ctor.instance_ = undefined;
  ctor.getInstance = function() {
    if (ctor.instance_) {
      return ctor.instance_;
    }
    if (goog.DEBUG) {
      // NOTE: JSCompiler can't optimize away Array#push.
      goog.instantiatedSingletons_[goog.instantiatedSingletons_.length] = ctor;
    }
    // Cast to avoid conformance violations that ctor.instance_ is unknown
    return /** @type {!Object|undefined} */ (ctor.instance_) = new ctor;
  };
};


/**
 * All singleton classes that have been instantiated, for testing. Don't read
 * it directly, use the `goog.testing.singleton` module. The compiler
 * removes this variable if unused.
 * @type {!Array<!Function>}
 * @private
 */
goog.instantiatedSingletons_ = [];


/**
 * @define {boolean} Whether to load goog.modules using `eval` when using
 * the debug loader.  This provides a better debugging experience as the
 * source is unmodified and can be edited using Chrome Workspaces or similar.
 * However in some environments the use of `eval` is banned
 * so we provide an alternative.
 */
goog.LOAD_MODULE_USING_EVAL = goog.define('goog.LOAD_MODULE_USING_EVAL', true);


/**
 * @define {boolean} Whether the exports of goog.modules should be sealed when
 * possible.
 */
goog.SEAL_MODULE_EXPORTS = goog.define('goog.SEAL_MODULE_EXPORTS', goog.DEBUG);


/**
 * The registry of initialized modules:
 * The module identifier or path to module exports map.
 * @private @const {!Object<string, {exports:?,type:string,moduleId:string}>}
 */
goog.loadedModules_ = {};


/**
 * True if the debug loader enabled and used.
 * @const {boolean}
 */
goog.DEPENDENCIES_ENABLED = !COMPILED && goog.ENABLE_DEBUG_LOADER;


/**
 * @define {string} How to decide whether to transpile.  Valid values
 * are 'always', 'never', and 'detect'.  The default ('detect') is to
 * use feature detection to determine which language levels need
 * transpilation.
 */
// NOTE(sdh): we could expand this to accept a language level to bypass
// detection: e.g. goog.TRANSPILE == 'es5' would transpile ES6 files but
// would leave ES3 and ES5 files alone.
goog.TRANSPILE = goog.define('goog.TRANSPILE', 'detect');

/**
 * @define {boolean} If true assume that ES modules have already been
 * transpiled by the jscompiler (in the same way that transpile.js would
 * transpile them - to jscomp modules). Useful only for servers that wish to use
 * the debug loader and transpile server side. Thus this is only respected if
 * goog.TRANSPILE is "never".
 */
goog.ASSUME_ES_MODULES_TRANSPILED =
    goog.define('goog.ASSUME_ES_MODULES_TRANSPILED', false);


/**
 * @define {string} If a file needs to be transpiled what the output language
 * should be. By default this is the highest language level this file detects
 * the current environment supports. Generally this flag should not be set, but
 * it could be useful to override. Example: If the current environment supports
 * ES6 then by default ES7+ files will be transpiled to ES6, unless this is
 * overridden.
 *
 * Valid values include: es3, es5, es6, es7, and es8. Anything not recognized
 * is treated as es3.
 *
 * Note that setting this value does not force transpilation. Just if
 * transpilation occurs this will be the output. So this is most useful when
 * goog.TRANSPILE is set to 'always' and then forcing the language level to be
 * something lower than what the environment detects.
 */
goog.TRANSPILE_TO_LANGUAGE = goog.define('goog.TRANSPILE_TO_LANGUAGE', '');


/**
 * @define {string} Path to the transpiler.  Executing the script at this
 * path (relative to base.js) should define a function $jscomp.transpile.
 */
goog.TRANSPILER = goog.define('goog.TRANSPILER', 'transpile.js');


/**
 * @define {string} Trusted Types policy name. If non-empty then Closure will
 * use Trusted Types.
 */
goog.TRUSTED_TYPES_POLICY_NAME =
    goog.define('goog.TRUSTED_TYPES_POLICY_NAME', 'goog');


/**
 * @package {?boolean}
 * Visible for testing.
 */
goog.hasBadLetScoping = null;


/**
 * @param {function(?):?|string} moduleDef The module definition.
 */
goog.loadModule = function(moduleDef) {
  // NOTE: we allow function definitions to be either in the from
  // of a string to eval (which keeps the original source intact) or
  // in a eval forbidden environment (CSP) we allow a function definition
  // which in its body must call `goog.module`, and return the exports
  // of the module.
  var previousState = goog.moduleLoaderState_;
  try {
    goog.moduleLoaderState_ = {
      moduleName: '',
      declareLegacyNamespace: false,
      type: goog.ModuleType.GOOG
    };
    var origExports = {};
    var exports = origExports;
    if (typeof moduleDef === 'function') {
      exports = moduleDef.call(undefined, exports);
    } else if (typeof moduleDef === 'string') {
      exports = goog.loadModuleFromSource_.call(undefined, exports, moduleDef);
    } else {
      throw new Error('Invalid module definition');
    }

    var moduleName = goog.moduleLoaderState_.moduleName;
    if (typeof moduleName === 'string' && moduleName) {
      // Don't seal legacy namespaces as they may be used as a parent of
      // another namespace
      if (goog.moduleLoaderState_.declareLegacyNamespace) {
        // Whether exports was overwritten via default export assignment.
        // This is important for legacy namespaces as it dictates whether
        // previously a previously loaded implicit namespace should be clobbered
        // or not.
        var isDefaultExport = origExports !== exports;
        goog.constructNamespace_(moduleName, exports, isDefaultExport);
      } else if (
          goog.SEAL_MODULE_EXPORTS && Object.seal &&
          typeof exports == 'object' && exports != null) {
        Object.seal(exports);
      }

      var data = {
        exports: exports,
        type: goog.ModuleType.GOOG,
        moduleId: goog.moduleLoaderState_.moduleName
      };
      goog.loadedModules_[moduleName] = data;
    } else {
      throw new Error('Invalid module name \"' + moduleName + '\"');
    }
  } finally {
    goog.moduleLoaderState_ = previousState;
  }
};


/**
 * @private @const
 */
goog.loadModuleFromSource_ =
    /** @type {function(!Object, string):?} */ (function(exports) {
      // NOTE: we avoid declaring parameters or local variables here to avoid
      // masking globals or leaking values into the module definition.
      'use strict';
      eval(goog.CLOSURE_EVAL_PREFILTER_.createScript(arguments[1]));
      return exports;
    });


/**
 * Normalize a file path by removing redundant ".." and extraneous "." file
 * path components.
 * @param {string} path
 * @return {string}
 * @private
 */
goog.normalizePath_ = function(path) {
  var components = path.split('/');
  var i = 0;
  while (i < components.length) {
    if (components[i] == '.') {
      components.splice(i, 1);
    } else if (
        i && components[i] == '..' && components[i - 1] &&
        components[i - 1] != '..') {
      components.splice(--i, 2);
    } else {
      i++;
    }
  }
  return components.join('/');
};


/**
 * Provides a hook for loading a file when using Closure's goog.require() API
 * with goog.modules.  In particular this hook is provided to support Node.js.
 *
 * @type {(function(string):string)|undefined}
 */
goog.global.CLOSURE_LOAD_FILE_SYNC;


/**
 * Loads file by synchronous XHR. Should not be used in production environments.
 * @param {string} src Source URL.
 * @return {?string} File contents, or null if load failed.
 * @private
 */
goog.loadFileSync_ = function(src) {
  if (goog.global.CLOSURE_LOAD_FILE_SYNC) {
    return goog.global.CLOSURE_LOAD_FILE_SYNC(src);
  } else {
    try {
      /** @type {XMLHttpRequest} */
      var xhr = new goog.global['XMLHttpRequest']();
      xhr.open('get', src, false);
      xhr.send();
      // NOTE: Successful http: requests have a status of 200, but successful
      // file: requests may have a status of zero.  Any other status, or a
      // thrown exception (particularly in case of file: requests) indicates
      // some sort of error, which we treat as a missing or unavailable file.
      return xhr.status == 0 || xhr.status == 200 ? xhr.responseText : null;
    } catch (err) {
      // No need to rethrow or log, since errors should show up on their own.
      return null;
    }
  }
};


/**
 * Lazily retrieves the transpiler and applies it to the source.
 * @param {string} code JS code.
 * @param {string} path Path to the code.
 * @param {string} target Language level output.
 * @return {string} The transpiled code.
 * @private
 */
goog.transpile_ = function(code, path, target) {
  var jscomp = goog.global['$jscomp'];
  if (!jscomp) {
    goog.global['$jscomp'] = jscomp = {};
  }
  var transpile = jscomp.transpile;
  if (!transpile) {
    var transpilerPath = goog.basePath + goog.TRANSPILER;
    var transpilerCode = goog.loadFileSync_(transpilerPath);
    if (transpilerCode) {
      // This must be executed synchronously, since by the time we know we
      // need it, we're about to load and write the ES6 code synchronously,
      // so a normal script-tag load will be too slow. Wrapped in a function
      // so that code is eval'd in the global scope.
      (function() {
        (0, eval)(transpilerCode + '\n//# sourceURL=' + transpilerPath);
      }).call(goog.global);
      // Even though the transpiler is optional, if $gwtExport is found, it's
      // a sign the transpiler was loaded and the $jscomp.transpile *should*
      // be there.
      if (goog.global['$gwtExport'] && goog.global['$gwtExport']['$jscomp'] &&
          !goog.global['$gwtExport']['$jscomp']['transpile']) {
        throw new Error(
            'The transpiler did not properly export the "transpile" ' +
            'method. $gwtExport: ' + JSON.stringify(goog.global['$gwtExport']));
      }
      // transpile.js only exports a single $jscomp function, transpile. We
      // grab just that and add it to the existing definition of $jscomp which
      // contains the polyfills.
      goog.global['$jscomp'].transpile =
          goog.global['$gwtExport']['$jscomp']['transpile'];
      jscomp = goog.global['$jscomp'];
      transpile = jscomp.transpile;
    }
  }
  if (!transpile) {
    // The transpiler is an optional component.  If it's not available then
    // replace it with a pass-through function that simply logs.
    var suffix = ' requires transpilation but no transpiler was found.';
    // MOE:begin_strip
    suffix +=  // Provide a more appropriate message internally.
        ' Please add "//javascript/closure:transpiler" as a data ' +
        'dependency to ensure it is included.';
    // MOE:end_strip
    transpile = jscomp.transpile = function(code, path) {
      // TODO(sdh): figure out some way to get this error to show up
      // in test results, noting that the failure may occur in many
      // different ways, including in loadModule() before the test
      // runner even comes up.
      goog.logToConsole_(path + suffix);
      return code;
    };
  }
  // Note: any transpilation errors/warnings will be logged to the console.
  return transpile(code, path, target);
};

//==============================================================================
// Language Enhancements
//==============================================================================


/**
 * This is a "fixed" version of the typeof operator.  It differs from the typeof
 * operator in such a way that null returns 'null' and arrays return 'array'.
 * @param {?} value The value to get the type of.
 * @return {string} The name of the type.
 */
goog.typeOf = function(value) {
  var s = typeof value;

  if (s != 'object') {
    return s;
  }

  if (!value) {
    return 'null';
  }

  if (Array.isArray(value)) {
    return 'array';
  }
  return s;
};


/**
 * Returns true if the object looks like an array. To qualify as array like
 * the value needs to be either a NodeList or an object with a Number length
 * property. Note that for this function neither strings nor functions are
 * considered "array-like".
 *
 * @param {?} val Variable to test.
 * @return {boolean} Whether variable is an array.
 */
goog.isArrayLike = function(val) {
  var type = goog.typeOf(val);
  // We do not use goog.isObject here in order to exclude function values.
  return type == 'array' || type == 'object' && typeof val.length == 'number';
};


/**
 * Returns true if the object looks like a Date. To qualify as Date-like the
 * value needs to be an object and have a getFullYear() function.
 * @param {?} val Variable to test.
 * @return {boolean} Whether variable is a like a Date.
 */
goog.isDateLike = function(val) {
  return goog.isObject(val) && typeof val.getFullYear == 'function';
};


/**
 * Returns true if the specified value is an object.  This includes arrays and
 * functions.
 * @param {?} val Variable to test.
 * @return {boolean} Whether variable is an object.
 */
goog.isObject = function(val) {
  var type = typeof val;
  return type == 'object' && val != null || type == 'function';
  // return Object(val) === val also works, but is slower, especially if val is
  // not an object.
};


/**
 * Gets a unique ID for an object. This mutates the object so that further calls
 * with the same object as a parameter returns the same value. The unique ID is
 * guaranteed to be unique across the current session amongst objects that are
 * passed into `getUid`. There is no guarantee that the ID is unique or
 * consistent across sessions. It is unsafe to generate unique ID for function
 * prototypes.
 *
 * @param {Object} obj The object to get the unique ID for.
 * @return {number} The unique ID for the object.
 */
goog.getUid = function(obj) {
  // TODO(arv): Make the type stricter, do not accept null.
  return Object.prototype.hasOwnProperty.call(obj, goog.UID_PROPERTY_) &&
      obj[goog.UID_PROPERTY_] ||
      (obj[goog.UID_PROPERTY_] = ++goog.uidCounter_);
};


/**
 * Whether the given object is already assigned a unique ID.
 *
 * This does not modify the object.
 *
 * @param {!Object} obj The object to check.
 * @return {boolean} Whether there is an assigned unique id for the object.
 */
goog.hasUid = function(obj) {
  return !!obj[goog.UID_PROPERTY_];
};


/**
 * Removes the unique ID from an object. This is useful if the object was
 * previously mutated using `goog.getUid` in which case the mutation is
 * undone.
 * @param {Object} obj The object to remove the unique ID field from.
 */
goog.removeUid = function(obj) {
  // TODO(arv): Make the type stricter, do not accept null.

  // In IE, DOM nodes are not instances of Object and throw an exception if we
  // try to delete.  Instead we try to use removeAttribute.
  if (obj !== null && 'removeAttribute' in obj) {
    obj.removeAttribute(goog.UID_PROPERTY_);
  }

  try {
    delete obj[goog.UID_PROPERTY_];
  } catch (ex) {
  }
};


/**
 * Name for unique ID property. Initialized in a way to help avoid collisions
 * with other closure JavaScript on the same page.
 * @type {string}
 * @private
 */
goog.UID_PROPERTY_ = 'closure_uid_' + ((Math.random() * 1e9) >>> 0);


/**
 * Counter for UID.
 * @type {number}
 * @private
 */
goog.uidCounter_ = 0;


/**
 * Clones a value. The input may be an Object, Array, or basic type. Objects and
 * arrays will be cloned recursively.
 *
 * WARNINGS:
 * <code>goog.cloneObject</code> does not detect reference loops. Objects that
 * refer to themselves will cause infinite recursion.
 *
 * <code>goog.cloneObject</code> is unaware of unique identifiers, and copies
 * UIDs created by <code>getUid</code> into cloned results.
 *
 * @param {*} obj The value to clone.
 * @return {*} A clone of the input value.
 * @deprecated goog.cloneObject is unsafe. Prefer the goog.object methods.
 */
goog.cloneObject = function(obj) {
  var type = goog.typeOf(obj);
  if (type == 'object' || type == 'array') {
    if (typeof obj.clone === 'function') {
      return obj.clone();
    }
    var clone = type == 'array' ? [] : {};
    for (var key in obj) {
      clone[key] = goog.cloneObject(obj[key]);
    }
    return clone;
  }

  return obj;
};


/**
 * A native implementation of goog.bind.
 * @param {?function(this:T, ...)} fn A function to partially apply.
 * @param {T} selfObj Specifies the object which this should point to when the
 *     function is run.
 * @param {...*} var_args Additional arguments that are partially applied to the
 *     function.
 * @return {!Function} A partially-applied form of the function goog.bind() was
 *     invoked as a method of.
 * @template T
 * @private
 */
goog.bindNative_ = function(fn, selfObj, var_args) {
  return /** @type {!Function} */ (fn.call.apply(fn.bind, arguments));
};


/**
 * A pure-JS implementation of goog.bind.
 * @param {?function(this:T, ...)} fn A function to partially apply.
 * @param {T} selfObj Specifies the object which this should point to when the
 *     function is run.
 * @param {...*} var_args Additional arguments that are partially applied to the
 *     function.
 * @return {!Function} A partially-applied form of the function goog.bind() was
 *     invoked as a method of.
 * @template T
 * @private
 */
goog.bindJs_ = function(fn, selfObj, var_args) {
  if (!fn) {
    throw new Error();
  }

  if (arguments.length > 2) {
    var boundArgs = Array.prototype.slice.call(arguments, 2);
    return function() {
      // Prepend the bound arguments to the current arguments.
      var newArgs = Array.prototype.slice.call(arguments);
      Array.prototype.unshift.apply(newArgs, boundArgs);
      return fn.apply(selfObj, newArgs);
    };

  } else {
    return function() {
      return fn.apply(selfObj, arguments);
    };
  }
};


/**
 * Partially applies this function to a particular 'this object' and zero or
 * more arguments. The result is a new function with some arguments of the first
 * function pre-filled and the value of this 'pre-specified'.
 *
 * Remaining arguments specified at call-time are appended to the pre-specified
 * ones.
 *
 * Also see: {@link #partial}.
 *
 * Usage:
 * <pre>var barMethBound = goog.bind(myFunction, myObj, 'arg1', 'arg2');
 * barMethBound('arg3', 'arg4');</pre>
 *
 * @param {?function(this:T, ...)} fn A function to partially apply.
 * @param {T} selfObj Specifies the object which this should point to when the
 *     function is run.
 * @param {...*} var_args Additional arguments that are partially applied to the
 *     function.
 * @return {!Function} A partially-applied form of the function goog.bind() was
 *     invoked as a method of.
 * @template T
 * @suppress {deprecated} See above.
 * @deprecated use `=> {}` or Function.prototype.bind instead.
 */
goog.bind = function(fn, selfObj, var_args) {
  // TODO(nicksantos): narrow the type signature.
  if (Function.prototype.bind &&
      // NOTE(nicksantos): Somebody pulled base.js into the default Chrome
      // extension environment. This means that for Chrome extensions, they get
      // the implementation of Function.prototype.bind that calls goog.bind
      // instead of the native one. Even worse, we don't want to introduce a
      // circular dependency between goog.bind and Function.prototype.bind, so
      // we have to hack this to make sure it works correctly.
      Function.prototype.bind.toString().indexOf('native code') != -1) {
    goog.bind = goog.bindNative_;
  } else {
    goog.bind = goog.bindJs_;
  }
  return goog.bind.apply(null, arguments);
};


/**
 * Like goog.bind(), except that a 'this object' is not required. Useful when
 * the target function is already bound.
 *
 * Usage:
 * var g = goog.partial(f, arg1, arg2);
 * g(arg3, arg4);
 *
 * @param {Function} fn A function to partially apply.
 * @param {...*} var_args Additional arguments that are partially applied to fn.
 * @return {!Function} A partially-applied form of the function goog.partial()
 *     was invoked as a method of.
 */
goog.partial = function(fn, var_args) {
  var args = Array.prototype.slice.call(arguments, 1);
  return function() {
    // Clone the array (with slice()) and append additional arguments
    // to the existing arguments.
    var newArgs = args.slice();
    newArgs.push.apply(newArgs, arguments);
    return fn.apply(/** @type {?} */ (this), newArgs);
  };
};


/**
 * Copies all the members of a source object to a target object. This method
 * does not work on all browsers for all objects that contain keys such as
 * toString or hasOwnProperty. Use goog.object.extend for this purpose.
 *
 * NOTE: Some have advocated for the use of goog.mixin to setup classes
 * with multiple inheritence (traits, mixins, etc).  However, as it simply
 * uses "for in", this is not compatible with ES6 classes whose methods are
 * non-enumerable.  Changing this, would break cases where non-enumerable
 * properties are not expected.
 *
 * @param {Object} target Target.
 * @param {Object} source Source.
 * @deprecated Prefer Object.assign
 */
goog.mixin = function(target, source) {
  for (var x in source) {
    target[x] = source[x];
  }

  // For IE7 or lower, the for-in-loop does not contain any properties that are
  // not enumerable on the prototype object (for example, isPrototypeOf from
  // Object.prototype) but also it will not include 'replace' on objects that
  // extend String and change 'replace' (not that it is common for anyone to
  // extend anything except Object).
};


/**
 * @return {number} An integer value representing the number of milliseconds
 *     between midnight, January 1, 1970 and the current time.
 * @deprecated Use Date.now
 */
goog.now = function() {
  return Date.now();
};


/**
 * Evals JavaScript in the global scope.
 *
 * Throws an exception if neither execScript or eval is defined.
 * @param {string|!TrustedScript} script JavaScript string.
 */
goog.globalEval = function(script) {
  (0, eval)(script);
};


/**
 * Optional map of CSS class names to obfuscated names used with
 * goog.getCssName().
 * @private {!Object<string, string>|undefined}
 * @see goog.setCssNameMapping
 */
goog.cssNameMapping_;


/**
 * Optional obfuscation style for CSS class names. Should be set to either
 * 'BY_WHOLE' or 'BY_PART' if defined.
 * @type {string|undefined}
 * @private
 * @see goog.setCssNameMapping
 */
goog.cssNameMappingStyle_;



/**
 * A hook for modifying the default behavior goog.getCssName. The function
 * if present, will receive the standard output of the goog.getCssName as
 * its input.
 *
 * @type {(function(string):string)|undefined}
 */
goog.global.CLOSURE_CSS_NAME_MAP_FN;


/**
 * Handles strings that are intended to be used as CSS class names.
 *
 * This function works in tandem with @see goog.setCssNameMapping.
 *
 * Without any mapping set, the arguments are simple joined with a hyphen and
 * passed through unaltered.
 *
 * When there is a mapping, there are two possible styles in which these
 * mappings are used. In the BY_PART style, each part (i.e. in between hyphens)
 * of the passed in css name is rewritten according to the map. In the BY_WHOLE
 * style, the full css name is looked up in the map directly. If a rewrite is
 * not specified by the map, the compiler will output a warning.
 *
 * When the mapping is passed to the compiler, it will replace calls to
 * goog.getCssName with the strings from the mapping, e.g.
 *     var x = goog.getCssName('foo');
 *     var y = goog.getCssName(this.baseClass, 'active');
 *  becomes:
 *     var x = 'foo';
 *     var y = this.baseClass + '-active';
 *
 * If one argument is passed it will be processed, if two are passed only the
 * modifier will be processed, as it is assumed the first argument was generated
 * as a result of calling goog.getCssName.
 *
 * @param {string} className The class name.
 * @param {string=} opt_modifier A modifier to be appended to the class name.
 * @return {string} The class name or the concatenation of the class name and
 *     the modifier.
 */
goog.getCssName = function(className, opt_modifier) {
  // String() is used for compatibility with compiled soy where the passed
  // className can be non-string objects.
  if (String(className).charAt(0) == '.') {
    throw new Error(
        'className passed in goog.getCssName must not start with ".".' +
        ' You passed: ' + className);
  }

  var getMapping = function(cssName) {
    return goog.cssNameMapping_[cssName] || cssName;
  };

  var renameByParts = function(cssName) {
    // Remap all the parts individually.
    var parts = cssName.split('-');
    var mapped = [];
    for (var i = 0; i < parts.length; i++) {
      mapped.push(getMapping(parts[i]));
    }
    return mapped.join('-');
  };

  var rename;
  if (goog.cssNameMapping_) {
    rename =
        goog.cssNameMappingStyle_ == 'BY_WHOLE' ? getMapping : renameByParts;
  } else {
    rename = function(a) {
      return a;
    };
  }

  var result =
      opt_modifier ? className + '-' + rename(opt_modifier) : rename(className);

  // The special CLOSURE_CSS_NAME_MAP_FN allows users to specify further
  // processing of the class name.
  if (goog.global.CLOSURE_CSS_NAME_MAP_FN) {
    return goog.global.CLOSURE_CSS_NAME_MAP_FN(result);
  }

  return result;
};


/**
 * Sets the map to check when returning a value from goog.getCssName(). Example:
 * <pre>
 * goog.setCssNameMapping({
 *   "goog": "a",
 *   "disabled": "b",
 * });
 *
 * var x = goog.getCssName('goog');
 * // The following evaluates to: "a a-b".
 * goog.getCssName('goog') + ' ' + goog.getCssName(x, 'disabled')
 * </pre>
 * When declared as a map of string literals to string literals, the JSCompiler
 * will replace all calls to goog.getCssName() using the supplied map if the
 * --process_closure_primitives flag is set.
 *
 * @param {!Object} mapping A map of strings to strings where keys are possible
 *     arguments to goog.getCssName() and values are the corresponding values
 *     that should be returned.
 * @param {string=} opt_style The style of css name mapping. There are two valid
 *     options: 'BY_PART', and 'BY_WHOLE'.
 * @see goog.getCssName for a description.
 */
goog.setCssNameMapping = function(mapping, opt_style) {
  goog.cssNameMapping_ = mapping;
  goog.cssNameMappingStyle_ = opt_style;
};


/**
 * To use CSS renaming in compiled mode, one of the input files should have a
 * call to goog.setCssNameMapping() with an object literal that the JSCompiler
 * can extract and use to replace all calls to goog.getCssName(). In uncompiled
 * mode, JavaScript code should be loaded before this base.js file that declares
 * a global variable, CLOSURE_CSS_NAME_MAPPING, which is used below. This is
 * to ensure that the mapping is loaded before any calls to goog.getCssName()
 * are made in uncompiled mode.
 *
 * A hook for overriding the CSS name mapping.
 * @type {!Object<string, string>|undefined}
 */
goog.global.CLOSURE_CSS_NAME_MAPPING;


if (!COMPILED && goog.global.CLOSURE_CSS_NAME_MAPPING) {
  // This does not call goog.setCssNameMapping() because the JSCompiler
  // requires that goog.setCssNameMapping() be called with an object literal.
  goog.cssNameMapping_ = goog.global.CLOSURE_CSS_NAME_MAPPING;
}


/**
 * Gets a localized message.
 *
 * This function is a compiler primitive. If you give the compiler a localized
 * message bundle, it will replace the string at compile-time with a localized
 * version, and expand goog.getMsg call to a concatenated string.
 *
 * Messages must be initialized in the form:
 * <code>
 * var MSG_NAME = goog.getMsg('Hello {$placeholder}', {'placeholder': 'world'});
 * </code>
 *
 * This function produces a string which should be treated as plain text. Use
 * {@link goog.html.SafeHtmlFormatter} in conjunction with goog.getMsg to
 * produce SafeHtml.
 *
 * @param {string} str Translatable string, places holders in the form {$foo}.
 * @param {Object<string, string>=} opt_values Maps place holder name to value.
 * @param {{html: (boolean|undefined),
 *         unescapeHtmlEntities: (boolean|undefined)}=} opt_options Options:
 *     html: Escape '<' in str to '&lt;'. Used by Closure Templates where the
 *     generated code size and performance is critical which is why {@link
 *     goog.html.SafeHtmlFormatter} is not used. The value must be literal true
 *     or false.
 *     unescapeHtmlEntities: Unescape common html entities: &gt;, &lt;, &apos;,
 *     &quot; and &amp;. Used for messages not in HTML context, such as with
 *     `textContent` property.
 * @return {string} message with placeholders filled.
 */
goog.getMsg = function(str, opt_values, opt_options) {
  if (opt_options && opt_options.html) {
    // Note that '&' is not replaced because the translation can contain HTML
    // entities.
    str = str.replace(/</g, '&lt;');
  }
  if (opt_options && opt_options.unescapeHtmlEntities) {
    // Note that "&amp;" must be the last to avoid "creating" new entities.
    str = str.replace(/&lt;/g, '<')
              .replace(/&gt;/g, '>')
              .replace(/&apos;/g, '\'')
              .replace(/&quot;/g, '"')
              .replace(/&amp;/g, '&');
  }
  if (opt_values) {
    str = str.replace(/\{\$([^}]+)}/g, function(match, key) {
      return (opt_values != null && key in opt_values) ? opt_values[key] :
                                                         match;
    });
  }
  return str;
};


/**
 * Gets a localized message. If the message does not have a translation, gives a
 * fallback message.
 *
 * This is useful when introducing a new message that has not yet been
 * translated into all languages.
 *
 * This function is a compiler primitive. Must be used in the form:
 * <code>var x = goog.getMsgWithFallback(MSG_A, MSG_B);</code>
 * where MSG_A and MSG_B were initialized with goog.getMsg.
 *
 * @param {string} a The preferred message.
 * @param {string} b The fallback message.
 * @return {string} The best translated message.
 */
goog.getMsgWithFallback = function(a, b) {
  return a;
};


/**
 * Exposes an unobfuscated global namespace path for the given object.
 * Note that fields of the exported object *will* be obfuscated, unless they are
 * exported in turn via this function or goog.exportProperty.
 *
 * Also handy for making public items that are defined in anonymous closures.
 *
 * ex. goog.exportSymbol('public.path.Foo', Foo);
 *
 * ex. goog.exportSymbol('public.path.Foo.staticFunction', Foo.staticFunction);
 *     public.path.Foo.staticFunction();
 *
 * ex. goog.exportSymbol('public.path.Foo.prototype.myMethod',
 *                       Foo.prototype.myMethod);
 *     new public.path.Foo().myMethod();
 *
 * @param {string} publicPath Unobfuscated name to export.
 * @param {*} object Object the name should point to.
 * @param {?Object=} objectToExportTo The object to add the path to; default
 *     is goog.global.
 */
goog.exportSymbol = function(publicPath, object, objectToExportTo) {
  goog.exportPath_(
      publicPath, object, /* overwriteImplicit= */ true, objectToExportTo);
};


/**
 * Exports a property unobfuscated into the object's namespace.
 * ex. goog.exportProperty(Foo, 'staticFunction', Foo.staticFunction);
 * ex. goog.exportProperty(Foo.prototype, 'myMethod', Foo.prototype.myMethod);
 * @param {Object} object Object whose static property is being exported.
 * @param {string} publicName Unobfuscated name to export.
 * @param {*} symbol Object the name should point to.
 */
goog.exportProperty = function(object, publicName, symbol) {
  object[publicName] = symbol;
};


/**
 * Inherit the prototype methods from one constructor into another.
 *
 * Usage:
 * <pre>
 * function ParentClass(a, b) { }
 * ParentClass.prototype.foo = function(a) { };
 *
 * function ChildClass(a, b, c) {
 *   ChildClass.base(this, 'constructor', a, b);
 * }
 * goog.inherits(ChildClass, ParentClass);
 *
 * var child = new ChildClass('a', 'b', 'see');
 * child.foo(); // This works.
 * </pre>
 *
 * @param {!Function} childCtor Child class.
 * @param {!Function} parentCtor Parent class.
 * @suppress {strictMissingProperties} superClass_ and base is not defined on
 *    Function.
 * @deprecated Use ECMAScript class syntax instead.
 */
goog.inherits = function(childCtor, parentCtor) {
  /** @constructor */
  function tempCtor() {}
  tempCtor.prototype = parentCtor.prototype;
  childCtor.superClass_ = parentCtor.prototype;
  childCtor.prototype = new tempCtor();
  /** @override */
  childCtor.prototype.constructor = childCtor;

  /**
   * Calls superclass constructor/method.
   *
   * This function is only available if you use goog.inherits to
   * express inheritance relationships between classes.
   *
   * NOTE: This is a replacement for goog.base and for superClass_
   * property defined in childCtor.
   *
   * @param {!Object} me Should always be "this".
   * @param {string} methodName The method name to call. Calling
   *     superclass constructor can be done with the special string
   *     'constructor'.
   * @param {...*} var_args The arguments to pass to superclass
   *     method/constructor.
   * @return {*} The return value of the superclass method/constructor.
   */
  childCtor.base = function(me, methodName, var_args) {
    // Copying using loop to avoid deop due to passing arguments object to
    // function. This is faster in many JS engines as of late 2014.
    var args = new Array(arguments.length - 2);
    for (var i = 2; i < arguments.length; i++) {
      args[i - 2] = arguments[i];
    }
    return parentCtor.prototype[methodName].apply(me, args);
  };
};


/**
 * Allow for aliasing within scope functions.  This function exists for
 * uncompiled code - in compiled code the calls will be inlined and the aliases
 * applied.  In uncompiled code the function is simply run since the aliases as
 * written are valid JavaScript.
 *
 * MOE:begin_intracomment_strip
 * See the goog.scope document at http://go/goog.scope
 *
 * For more on goog.scope deprecation, see the style guide entry:
 * http://go/jsstyle#appendices-legacy-exceptions-goog-scope
 * MOE:end_intracomment_strip
 *
 * @param {function()} fn Function to call.  This function can contain aliases
 *     to namespaces (e.g. "var dom = goog.dom") or classes
 *     (e.g. "var Timer = goog.Timer").
 * @deprecated Use goog.module instead.
 */
goog.scope = function(fn) {
  if (goog.isInModuleLoader_()) {
    throw new Error('goog.scope is not supported within a module.');
  }
  fn.call(goog.global);
};


/*
 * To support uncompiled, strict mode bundles that use eval to divide source
 * like so:
 *    eval('someSource;//# sourceUrl sourcefile.js');
 * We need to export the globally defined symbols "goog" and "COMPILED".
 * Exporting "goog" breaks the compiler optimizations, so we required that
 * be defined externally.
 * NOTE: We don't use goog.exportSymbol here because we don't want to trigger
 * extern generation when that compiler option is enabled.
 */
if (!COMPILED) {
  goog.global['COMPILED'] = COMPILED;
}


//==============================================================================
// goog.defineClass implementation
//==============================================================================


/**
 * Creates a restricted form of a Closure "class":
 *   - from the compiler's perspective, the instance returned from the
 *     constructor is sealed (no new properties may be added).  This enables
 *     better checks.
 *   - the compiler will rewrite this definition to a form that is optimal
 *     for type checking and optimization (initially this will be a more
 *     traditional form).
 *
 * @param {Function} superClass The superclass, Object or null.
 * @param {goog.defineClass.ClassDescriptor} def
 *     An object literal describing
 *     the class.  It may have the following properties:
 *     "constructor": the constructor function
 *     "statics": an object literal containing methods to add to the constructor
 *        as "static" methods or a function that will receive the constructor
 *        function as its only parameter to which static properties can
 *        be added.
 *     all other properties are added to the prototype.
 * @return {!Function} The class constructor.
 * @deprecated Use ECMAScript class syntax instead.
 */
goog.defineClass = function(superClass, def) {
  // TODO(johnlenz): consider making the superClass an optional parameter.
  var constructor = def.constructor;
  var statics = def.statics;
  // Wrap the constructor prior to setting up the prototype and static methods.
  if (!constructor || constructor == Object.prototype.constructor) {
    constructor = function() {
      throw new Error(
          'cannot instantiate an interface (no constructor defined).');
    };
  }

  var cls = goog.defineClass.createSealingConstructor_(constructor, superClass);
  if (superClass) {
    goog.inherits(cls, superClass);
  }

  // Remove all the properties that should not be copied to the prototype.
  delete def.constructor;
  delete def.statics;

  goog.defineClass.applyProperties_(cls.prototype, def);
  if (statics != null) {
    if (statics instanceof Function) {
      statics(cls);
    } else {
      goog.defineClass.applyProperties_(cls, statics);
    }
  }

  return cls;
};


/**
 * @typedef {{
 *   constructor: (!Function|undefined),
 *   statics: (Object|undefined|function(Function):void)
 * }}
 */
goog.defineClass.ClassDescriptor;


/**
 * @define {boolean} Whether the instances returned by goog.defineClass should
 *     be sealed when possible.
 *
 * When sealing is disabled the constructor function will not be wrapped by
 * goog.defineClass, making it incompatible with ES6 class methods.
 */
goog.defineClass.SEAL_CLASS_INSTANCES =
    goog.define('goog.defineClass.SEAL_CLASS_INSTANCES', goog.DEBUG);


/**
 * If goog.defineClass.SEAL_CLASS_INSTANCES is enabled and Object.seal is
 * defined, this function will wrap the constructor in a function that seals the
 * results of the provided constructor function.
 *
 * @param {!Function} ctr The constructor whose results maybe be sealed.
 * @param {Function} superClass The superclass constructor.
 * @return {!Function} The replacement constructor.
 * @private
 */
goog.defineClass.createSealingConstructor_ = function(ctr, superClass) {
  if (!goog.defineClass.SEAL_CLASS_INSTANCES) {
    // Do now wrap the constructor when sealing is disabled. Angular code
    // depends on this for injection to work properly.
    return ctr;
  }

  // NOTE: The sealing behavior has been removed

  /**
   * @this {Object}
   * @return {?}
   */
  var wrappedCtr = function() {
    // Don't seal an instance of a subclass when it calls the constructor of
    // its super class as there is most likely still setup to do.
    var instance = ctr.apply(this, arguments) || this;
    instance[goog.UID_PROPERTY_] = instance[goog.UID_PROPERTY_];

    return instance;
  };

  return wrappedCtr;
};



// TODO(johnlenz): share these values with the goog.object
/**
 * The names of the fields that are defined on Object.prototype.
 * @type {!Array<string>}
 * @private
 * @const
 */
goog.defineClass.OBJECT_PROTOTYPE_FIELDS_ = [
  'constructor', 'hasOwnProperty', 'isPrototypeOf', 'propertyIsEnumerable',
  'toLocaleString', 'toString', 'valueOf'
];


// TODO(johnlenz): share this function with the goog.object
/**
 * @param {!Object} target The object to add properties to.
 * @param {!Object} source The object to copy properties from.
 * @private
 */
goog.defineClass.applyProperties_ = function(target, source) {
  // TODO(johnlenz): update this to support ES5 getters/setters

  var key;
  for (key in source) {
    if (Object.prototype.hasOwnProperty.call(source, key)) {
      target[key] = source[key];
    }
  }

  // For IE the for-in-loop does not contain any properties that are not
  // enumerable on the prototype object (for example isPrototypeOf from
  // Object.prototype) and it will also not include 'replace' on objects that
  // extend String and change 'replace' (not that it is common for anyone to
  // extend anything except Object).
  for (var i = 0; i < goog.defineClass.OBJECT_PROTOTYPE_FIELDS_.length; i++) {
    key = goog.defineClass.OBJECT_PROTOTYPE_FIELDS_[i];
    if (Object.prototype.hasOwnProperty.call(source, key)) {
      target[key] = source[key];
    }
  }
};

/**
 * Returns the parameter.
 * @param {string} s
 * @return {string}
 * @private
 */
goog.identity_ = function(s) {
  return s;
};


/**
 * Creates Trusted Types policy if Trusted Types are supported by the browser.
 * The policy just blesses any string as a Trusted Type. It is not visibility
 * restricted because anyone can also call trustedTypes.createPolicy directly.
 * However, the allowed names should be restricted by a HTTP header and the
 * reference to the created policy should be visibility restricted.
 * @param {string} name
 * @return {?TrustedTypePolicy}
 */
goog.createTrustedTypesPolicy = function(name) {
  var policy = null;
  var policyFactory = goog.global.trustedTypes;
  if (!policyFactory || !policyFactory.createPolicy) {
    return policy;
  }
  // trustedTypes.createPolicy throws if called with a name that is already
  // registered, even in report-only mode. Until the API changes, catch the
  // error not to break the applications functionally. In such case, the code
  // will fall back to using regular Safe Types.
  // TODO(koto): Remove catching once createPolicy API stops throwing.
  try {
    policy = policyFactory.createPolicy(name, {
      createHTML: goog.identity_,
      createScript: goog.identity_,
      createScriptURL: goog.identity_
    });
  } catch (e) {
    goog.logToConsole_(e.message);
  }
  return policy;
};

// There's a bug in the compiler where without collapse properties the
// Closure namespace defines do not guard code correctly. To help reduce code
// size also check for !COMPILED even though it redundant until this is fixed.
if (!COMPILED && goog.DEPENDENCIES_ENABLED) {
  // MOE:begin_strip
  // TODO(b/67050526) This object is obsolete but some people are relying on
  // it internally. Keep it around until we migrate them.
  /**
   * @private
   * @type {{
   *   loadFlags: !Object<string, !Object<string, string>>,
   *   nameToPath: !Object<string, string>,
   *   requires: !Object<string, !Object<string, boolean>>,
   *   visited: !Object<string, boolean>,
   *   written: !Object<string, boolean>,
   *   deferred: !Object<string, string>
   * }}
   */
  goog.dependencies_ = {
    loadFlags: {},  // 1 to 1

    nameToPath: {},  // 1 to 1

    requires: {},  // 1 to many

    // Used when resolving dependencies to prevent us from visiting file
    // twice.
    visited: {},

    written: {},  // Used to keep track of script files we have written.

    deferred: {}  // Used to track deferred module evaluations in old IEs
  };

  /**
   * @return {!Object}
   * @private
   */
  goog.getLoader_ = function() {
    return {
      dependencies_: goog.dependencies_,
      writeScriptTag_: goog.writeScriptTag_
    };
  };


  /**
   * @param {string} src The script url.
   * @param {string=} opt_sourceText The optionally source text to evaluate
   * @return {boolean} True if the script was imported, false otherwise.
   * @private
   */
  goog.writeScriptTag_ = function(src, opt_sourceText) {
    if (goog.inHtmlDocument_()) {
      /** @type {!HTMLDocument} */
      var doc = goog.global.document;

      // If the user tries to require a new symbol after document load,
      // something has gone terribly wrong. Doing a document.write would
      // wipe out the page. This does not apply to the CSP-compliant method
      // of writing script tags.
      if (!goog.ENABLE_CHROME_APP_SAFE_SCRIPT_LOADING &&
          doc.readyState == 'complete') {
        // Certain test frameworks load base.js multiple times, which tries
        // to write deps.js each time. If that happens, just fail silently.
        // These frameworks wipe the page between each load of base.js, so this
        // is OK.
        var isDeps = /\bdeps.js$/.test(src);
        if (isDeps) {
          return false;
        } else {
          throw Error('Cannot write "' + src + '" after document load');
        }
      }

      var nonceAttr = '';
      var nonce = goog.getScriptNonce();
      if (nonce) {
        nonceAttr = ' nonce="' + nonce + '"';
      }

      if (opt_sourceText === undefined) {
        var script = '<script src="' + src + '"' + nonceAttr + '></' +
            'script>';
        doc.write(
            goog.TRUSTED_TYPES_POLICY_ ?
                goog.TRUSTED_TYPES_POLICY_.createHTML(script) :
                script);
      } else {
        var script = '<script' + nonceAttr + '>' +
            goog.protectScriptTag_(opt_sourceText) + '</' +
            'script>';
        doc.write(
            goog.TRUSTED_TYPES_POLICY_ ?
                goog.TRUSTED_TYPES_POLICY_.createHTML(script) :
                script);
      }
      return true;
    } else {
      return false;
    }
  };
  // MOE:end_strip


  /**
   * Tries to detect whether the current browser is Edge, based on the user
   * agent. This matches only pre-Chromium Edge.
   * @see https://docs.microsoft.com/en-us/microsoft-edge/web-platform/user-agent-string
   * @return {boolean} True if the current browser is Edge.
   * @private
   */
  goog.isEdge_ = function() {
    var userAgent = goog.global.navigator && goog.global.navigator.userAgent ?
        goog.global.navigator.userAgent :
        '';
    var edgeRe = /Edge\/(\d+)(\.\d)*/i;
    return !!userAgent.match(edgeRe);
  };


  /**
   * Tries to detect whether is in the context of an HTML document.
   * @return {boolean} True if it looks like HTML document.
   * @private
   */
  goog.inHtmlDocument_ = function() {
    /** @type {!Document} */
    var doc = goog.global.document;
    return doc != null && 'write' in doc;  // XULDocument misses write.
  };


  /**
   * We'd like to check for if the document readyState is 'loading'; however
   * there are bugs on IE 10 and below where the readyState being anything other
   * than 'complete' is not reliable.
   * @return {boolean}
   * @private
   */
  goog.isDocumentLoading_ = function() {
    // attachEvent is available on IE 6 thru 10 only, and thus can be used to
    // detect those browsers.
    /** @type {!HTMLDocument} */
    var doc = goog.global.document;
    return doc.attachEvent ? doc.readyState != 'complete' :
                             doc.readyState == 'loading';
  };


  /**
   * Tries to detect the base path of base.js script that bootstraps Closure.
   * @private
   */
  goog.findBasePath_ = function() {
    if (goog.global.CLOSURE_BASE_PATH != undefined &&
        // Anti DOM-clobbering runtime check (b/37736576).
        typeof goog.global.CLOSURE_BASE_PATH === 'string') {
      goog.basePath = goog.global.CLOSURE_BASE_PATH;
      return;
    } else if (!goog.inHtmlDocument_()) {
      return;
    }
    /** @type {!Document} */
    var doc = goog.global.document;
    // If we have a currentScript available, use it exclusively.
    var currentScript = doc.currentScript;
    if (currentScript) {
      var scripts = [currentScript];
    } else {
      var scripts = doc.getElementsByTagName('SCRIPT');
    }
    // Search backwards since the current script is in almost all cases the one
    // that has base.js.
    for (var i = scripts.length - 1; i >= 0; --i) {
      var script = /** @type {!HTMLScriptElement} */ (scripts[i]);
      var src = script.src;
      var qmark = src.lastIndexOf('?');
      var l = qmark == -1 ? src.length : qmark;
      if (src.substr(l - 7, 7) == 'base.js') {
        goog.basePath = src.substr(0, l - 7);
        return;
      }
    }
  };

  goog.findBasePath_();

  /** @struct @constructor @final */
  goog.Transpiler = function() {
    /** @private {?Object<string, boolean>} */
    this.requiresTranspilation_ = null;
    /** @private {string} */
    this.transpilationTarget_ = goog.TRANSPILE_TO_LANGUAGE;
  };


  // MOE:begin_strip
  // LINT.IfChange
  // MOE:end_strip
  /**
   * Returns a newly created map from language mode string to a boolean
   * indicating whether transpilation should be done for that mode as well as
   * the highest level language that this environment supports.
   *
   * Guaranteed invariant:
   * For any two modes, l1 and l2 where l2 is a newer mode than l1,
   * `map[l1] == true` implies that `map[l2] == true`.
   *
   * Note this method is extracted and used elsewhere, so it cannot rely on
   * anything external (it should easily be able to be transformed into a
   * standalone, top level function).
   *
   * @private
   * @return {{
   *   target: string,
   *   map: !Object<string, boolean>
   * }}
   */
  goog.Transpiler.prototype.createRequiresTranspilation_ = function() {
    var transpilationTarget = 'es3';
    var /** !Object<string, boolean> */ requiresTranspilation = {'es3': false};
    var transpilationRequiredForAllLaterModes = false;

    /**
     * Adds an entry to requiresTranspliation for the given language mode.
     *
     * IMPORTANT: Calls must be made in order from oldest to newest language
     * mode.
     * @param {string} modeName
     * @param {function(): boolean} isSupported Returns true if the JS engine
     *     supports the given mode.
     */
    function addNewerLanguageTranspilationCheck(modeName, isSupported) {
      if (transpilationRequiredForAllLaterModes) {
        requiresTranspilation[modeName] = true;
      } else if (isSupported()) {
        transpilationTarget = modeName;
        requiresTranspilation[modeName] = false;
      } else {
        requiresTranspilation[modeName] = true;
        transpilationRequiredForAllLaterModes = true;
      }
    }

    /**
     * Does the given code evaluate without syntax errors and return a truthy
     * result?
     */
    function /** boolean */ evalCheck(/** string */ code) {
      try {
        return !!eval(goog.CLOSURE_EVAL_PREFILTER_.createScript(code));
      } catch (ignored) {
        return false;
      }
    }

    // Identify ES3-only browsers by their incorrect treatment of commas.
    addNewerLanguageTranspilationCheck('es5', function() {
      return evalCheck('[1,].length==1');
    });
    addNewerLanguageTranspilationCheck('es6', function() {
      // Edge has a non-deterministic (i.e., not reproducible) bug with ES6:
      // https://github.com/Microsoft/ChakraCore/issues/1496.
      // MOE:begin_strip
      // TODO(joeltine): Our internal web-testing version of Edge will need to
      // be updated before we can remove this check. See http://b/34945376.
      // MOE:end_strip
      if (goog.isEdge_()) {
        // The Reflect.construct test below is flaky on Edge. It can sometimes
        // pass or fail on 40 15.15063, so just exit early for Edge and treat
        // it as ES5. Until we're on a more up to date version just always use
        // ES5. See https://github.com/Microsoft/ChakraCore/issues/3217.
        return false;
      }
      // Test es6: [FF50 (?), Edge 14 (?), Chrome 50]
      //   (a) default params (specifically shadowing locals),
      //   (b) destructuring, (c) block-scoped functions,
      //   (d) for-of (const), (e) new.target/Reflect.construct
      var es6fullTest =
          'class X{constructor(){if(new.target!=String)throw 1;this.x=42}}' +
          'let q=Reflect.construct(X,[],String);if(q.x!=42||!(q instanceof ' +
          'String))throw 1;for(const a of[2,3]){if(a==2)continue;function ' +
          'f(z={a}){let a=0;return z.a}{function f(){return 0;}}return f()' +
          '==3}';

      return evalCheck('(()=>{"use strict";' + es6fullTest + '})()');
    });
    // ** and **= are the only new features in 'es7'
    addNewerLanguageTranspilationCheck('es7', function() {
      return evalCheck('2**3==8');
    });
    // async functions are the only new features in 'es8'
    addNewerLanguageTranspilationCheck('es8', function() {
      return evalCheck('async()=>1,1');
    });
    addNewerLanguageTranspilationCheck('es9', function() {
      return evalCheck('({...rest}={}),1');
    });
    // optional catch binding, unescaped unicode paragraph separator in strings
    addNewerLanguageTranspilationCheck('es_2019', function() {
      return evalCheck('let r;try{throw 0}catch{r="\u2029"};r');
    });
    // optional chaining, nullish coalescing
    // untested/unsupported: bigint, import meta
    addNewerLanguageTranspilationCheck('es_2020', function() {
      return evalCheck('null?.x??1');
    });
    addNewerLanguageTranspilationCheck('es_next', function() {
      return false;  // assume it always need to transpile
    });
    return {target: transpilationTarget, map: requiresTranspilation};
  };
  // MOE:begin_strip
  // LINT.ThenChange(//depot/google3/java/com/google/testing/web/devtools/updatebrowserinfo/requires_transpilation.js)
  // MOE:end_strip


  /**
   * Determines whether the given language needs to be transpiled.
   * @param {string} lang
   * @param {string|undefined} module
   * @return {boolean}
   */
  goog.Transpiler.prototype.needsTranspile = function(lang, module) {
    if (goog.TRANSPILE == 'always') {
      return true;
    } else if (goog.TRANSPILE == 'never') {
      return false;
    } else if (!this.requiresTranspilation_) {
      var obj = this.createRequiresTranspilation_();
      this.requiresTranspilation_ = obj.map;
      this.transpilationTarget_ = this.transpilationTarget_ || obj.target;
    }
    if (lang in this.requiresTranspilation_) {
      if (this.requiresTranspilation_[lang]) {
        return true;
      } else if (
          goog.inHtmlDocument_() && module == 'es6' &&
          !('noModule' in goog.global.document.createElement('script'))) {
        return true;
      } else {
        return false;
      }
    } else {
      throw new Error('Unknown language mode: ' + lang);
    }
  };


  /**
   * Lazily retrieves the transpiler and applies it to the source.
   * @param {string} code JS code.
   * @param {string} path Path to the code.
   * @return {string} The transpiled code.
   */
  goog.Transpiler.prototype.transpile = function(code, path) {
    // TODO(johnplaisted): We should delete goog.transpile_ and just have this
    // function. But there's some compile error atm where goog.global is being
    // stripped incorrectly without this.
    return goog.transpile_(code, path, this.transpilationTarget_);
  };


  /** @private @final {!goog.Transpiler} */
  goog.transpiler_ = new goog.Transpiler();

  /**
   * Rewrites closing script tags in input to avoid ending an enclosing script
   * tag.
   *
   * @param {string} str
   * @return {string}
   * @private
   */
  goog.protectScriptTag_ = function(str) {
    return str.replace(/<\/(SCRIPT)/ig, '\\x3c/$1');
  };


  /**
   * A debug loader is responsible for downloading and executing javascript
   * files in an unbundled, uncompiled environment.
   *
   * This can be custimized via the setDependencyFactory method, or by
   * CLOSURE_IMPORT_SCRIPT/CLOSURE_LOAD_FILE_SYNC.
   *
   * @struct @constructor @final @private
   */
  goog.DebugLoader_ = function() {
    /** @private @const {!Object<string, !goog.Dependency>} */
    this.dependencies_ = {};
    /** @private @const {!Object<string, string>} */
    this.idToPath_ = {};
    /** @private @const {!Object<string, boolean>} */
    this.written_ = {};
    /** @private @const {!Array<!goog.Dependency>} */
    this.loadingDeps_ = [];
    /** @private {!Array<!goog.Dependency>} */
    this.depsToLoad_ = [];
    /** @private {boolean} */
    this.paused_ = false;
    /** @private {!goog.DependencyFactory} */
    this.factory_ = new goog.DependencyFactory(goog.transpiler_);
    /** @private @const {!Object<string, !Function>} */
    this.deferredCallbacks_ = {};
    /** @private @const {!Array<string>} */
    this.deferredQueue_ = [];
  };

  /**
   * @param {!Array<string>} namespaces
   * @param {function(): undefined} callback Function to call once all the
   *     namespaces have loaded.
   */
  goog.DebugLoader_.prototype.bootstrap = function(namespaces, callback) {
    var cb = callback;
    function resolve() {
      if (cb) {
        goog.global.setTimeout(cb, 0);
        cb = null;
      }
    }

    if (!namespaces.length) {
      resolve();
      return;
    }

    var deps = [];
    for (var i = 0; i < namespaces.length; i++) {
      var path = this.getPathFromDeps_(namespaces[i]);
      if (!path) {
        throw new Error('Unregonized namespace: ' + namespaces[i]);
      }
      deps.push(this.dependencies_[path]);
    }

    var require = goog.require;
    var loaded = 0;
    for (var i = 0; i < namespaces.length; i++) {
      require(namespaces[i]);
      deps[i].onLoad(function() {
        if (++loaded == namespaces.length) {
          resolve();
        }
      });
    }
  };


  /**
   * Loads the Closure Dependency file.
   *
   * Exposed a public function so CLOSURE_NO_DEPS can be set to false, base
   * loaded, setDependencyFactory called, and then this called. i.e. allows
   * custom loading of the deps file.
   */
  goog.DebugLoader_.prototype.loadClosureDeps = function() {
    // Circumvent addDependency, which would try to transpile deps.js if
    // transpile is set to always.
    var relPath = 'deps.js';
    this.depsToLoad_.push(this.factory_.createDependency(
        goog.normalizePath_(goog.basePath + relPath), relPath, [], [], {},
        false));
    this.loadDeps_();
  };


  /**
   * Notifies the debug loader when a dependency has been requested.
   *
   * @param {string} absPathOrId Path of the dependency or goog id.
   * @param {boolean=} opt_force
   */
  goog.DebugLoader_.prototype.requested = function(absPathOrId, opt_force) {
    var path = this.getPathFromDeps_(absPathOrId);
    if (path &&
        (opt_force || this.areDepsLoaded_(this.dependencies_[path].requires))) {
      var callback = this.deferredCallbacks_[path];
      if (callback) {
        delete this.deferredCallbacks_[path];
        callback();
      }
    }
  };


  /**
   * Sets the dependency factory, which can be used to create custom
   * goog.Dependency implementations to control how dependencies are loaded.
   *
   * @param {!goog.DependencyFactory} factory
   */
  goog.DebugLoader_.prototype.setDependencyFactory = function(factory) {
    this.factory_ = factory;
  };


  /**
   * Travserses the dependency graph and queues the given dependency, and all of
   * its transitive dependencies, for loading and then starts loading if not
   * paused.
   *
   * @param {string} namespace
   * @private
   */
  goog.DebugLoader_.prototype.load_ = function(namespace) {
    if (!this.getPathFromDeps_(namespace)) {
      var errorMessage = 'goog.require could not find: ' + namespace;
      goog.logToConsole_(errorMessage);
    } else {
      var loader = this;

      var deps = [];

      /** @param {string} namespace */
      var visit = function(namespace) {
        var path = loader.getPathFromDeps_(namespace);

        if (!path) {
          throw new Error('Bad dependency path or symbol: ' + namespace);
        }

        if (loader.written_[path]) {
          return;
        }

        loader.written_[path] = true;

        var dep = loader.dependencies_[path];
        // MOE:begin_strip
        if (goog.dependencies_.written[dep.relativePath]) {
          return;
        }
        // MOE:end_strip
        for (var i = 0; i < dep.requires.length; i++) {
          if (!goog.isProvided_(dep.requires[i])) {
            visit(dep.requires[i]);
          }
        }

        deps.push(dep);
      };

      visit(namespace);

      var wasLoading = !!this.depsToLoad_.length;
      this.depsToLoad_ = this.depsToLoad_.concat(deps);

      if (!this.paused_ && !wasLoading) {
        this.loadDeps_();
      }
    }
  };


  /**
   * Loads any queued dependencies until they are all loaded or paused.
   *
   * @private
   */
  goog.DebugLoader_.prototype.loadDeps_ = function() {
    var loader = this;
    var paused = this.paused_;

    while (this.depsToLoad_.length && !paused) {
      (function() {
        var loadCallDone = false;
        var dep = loader.depsToLoad_.shift();

        var loaded = false;
        loader.loading_(dep);

        var controller = {
          pause: function() {
            if (loadCallDone) {
              throw new Error('Cannot call pause after the call to load.');
            } else {
              paused = true;
            }
          },
          resume: function() {
            if (loadCallDone) {
              loader.resume_();
            } else {
              // Some dep called pause and then resume in the same load call.
              // Just keep running this same loop.
              paused = false;
            }
          },
          loaded: function() {
            if (loaded) {
              throw new Error('Double call to loaded.');
            }

            loaded = true;
            loader.loaded_(dep);
          },
          pending: function() {
            // Defensive copy.
            var pending = [];
            for (var i = 0; i < loader.loadingDeps_.length; i++) {
              pending.push(loader.loadingDeps_[i]);
            }
            return pending;
          },
          /**
           * @param {goog.ModuleType} type
           */
          setModuleState: function(type) {
            goog.moduleLoaderState_ = {
              type: type,
              moduleName: '',
              declareLegacyNamespace: false
            };
          },
          /** @type {function(string, string, string=)} */
          registerEs6ModuleExports: function(
              path, exports, opt_closureNamespace) {
            if (opt_closureNamespace) {
              goog.loadedModules_[opt_closureNamespace] = {
                exports: exports,
                type: goog.ModuleType.ES6,
                moduleId: opt_closureNamespace || ''
              };
            }
          },
          /** @type {function(string, ?)} */
          registerGoogModuleExports: function(moduleId, exports) {
            goog.loadedModules_[moduleId] = {
              exports: exports,
              type: goog.ModuleType.GOOG,
              moduleId: moduleId
            };
          },
          clearModuleState: function() {
            goog.moduleLoaderState_ = null;
          },
          defer: function(callback) {
            if (loadCallDone) {
              throw new Error(
                  'Cannot register with defer after the call to load.');
            }
            loader.defer_(dep, callback);
          },
          areDepsLoaded: function() {
            return loader.areDepsLoaded_(dep.requires);
          }
        };

        try {
          dep.load(controller);
        } finally {
          loadCallDone = true;
        }
      })();
    }

    if (paused) {
      this.pause_();
    }
  };


  /** @private */
  goog.DebugLoader_.prototype.pause_ = function() {
    this.paused_ = true;
  };


  /** @private */
  goog.DebugLoader_.prototype.resume_ = function() {
    if (this.paused_) {
      this.paused_ = false;
      this.loadDeps_();
    }
  };


  /**
   * Marks the given dependency as loading (load has been called but it has not
   * yet marked itself as finished). Useful for dependencies that want to know
   * what else is loading. Example: goog.modules cannot eval if there are
   * loading dependencies.
   *
   * @param {!goog.Dependency} dep
   * @private
   */
  goog.DebugLoader_.prototype.loading_ = function(dep) {
    this.loadingDeps_.push(dep);
  };


  /**
   * Marks the given dependency as having finished loading and being available
   * for require.
   *
   * @param {!goog.Dependency} dep
   * @private
   */
  goog.DebugLoader_.prototype.loaded_ = function(dep) {
    for (var i = 0; i < this.loadingDeps_.length; i++) {
      if (this.loadingDeps_[i] == dep) {
        this.loadingDeps_.splice(i, 1);
        break;
      }
    }

    for (var i = 0; i < this.deferredQueue_.length; i++) {
      if (this.deferredQueue_[i] == dep.path) {
        this.deferredQueue_.splice(i, 1);
        break;
      }
    }

    if (this.loadingDeps_.length == this.deferredQueue_.length &&
        !this.depsToLoad_.length) {
      // Something has asked to load these, but they may not be directly
      // required again later, so load them now that we know we're done loading
      // everything else. e.g. a goog module entry point.
      while (this.deferredQueue_.length) {
        this.requested(this.deferredQueue_.shift(), true);
      }
    }

    dep.loaded();
  };


  /**
   * @param {!Array<string>} pathsOrIds
   * @return {boolean}
   * @private
   */
  goog.DebugLoader_.prototype.areDepsLoaded_ = function(pathsOrIds) {
    for (var i = 0; i < pathsOrIds.length; i++) {
      var path = this.getPathFromDeps_(pathsOrIds[i]);
      if (!path ||
          (!(path in this.deferredCallbacks_) &&
           !goog.isProvided_(pathsOrIds[i]))) {
        return false;
      }
    }

    return true;
  };


  /**
   * @param {string} absPathOrId
   * @return {?string}
   * @private
   */
  goog.DebugLoader_.prototype.getPathFromDeps_ = function(absPathOrId) {
    if (absPathOrId in this.idToPath_) {
      return this.idToPath_[absPathOrId];
    } else if (absPathOrId in this.dependencies_) {
      return absPathOrId;
    } else {
      return null;
    }
  };


  /**
   * @param {!goog.Dependency} dependency
   * @param {!Function} callback
   * @private
   */
  goog.DebugLoader_.prototype.defer_ = function(dependency, callback) {
    this.deferredCallbacks_[dependency.path] = callback;
    this.deferredQueue_.push(dependency.path);
  };


  /**
   * Interface for goog.Dependency implementations to have some control over
   * loading of dependencies.
   *
   * @record
   */
  goog.LoadController = function() {};


  /**
   * Tells the controller to halt loading of more dependencies.
   */
  goog.LoadController.prototype.pause = function() {};


  /**
   * Tells the controller to resume loading of more dependencies if paused.
   */
  goog.LoadController.prototype.resume = function() {};


  /**
   * Tells the controller that this dependency has finished loading.
   *
   * This causes this to be removed from pending() and any load callbacks to
   * fire.
   */
  goog.LoadController.prototype.loaded = function() {};


  /**
   * List of dependencies on which load has been called but which have not
   * called loaded on their controller. This includes the current dependency.
   *
   * @return {!Array<!goog.Dependency>}
   */
  goog.LoadController.prototype.pending = function() {};


  /**
   * Registers an object as an ES6 module's exports so that goog.modules may
   * require it by path.
   *
   * @param {string} path Full path of the module.
   * @param {?} exports
   * @param {string=} opt_closureNamespace Closure namespace to associate with
   *     this module.
   */
  goog.LoadController.prototype.registerEs6ModuleExports = function(
      path, exports, opt_closureNamespace) {};


  /**
   * Sets the current module state.
   *
   * @param {goog.ModuleType} type Type of module.
   */
  goog.LoadController.prototype.setModuleState = function(type) {};


  /**
   * Clears the current module state.
   */
  goog.LoadController.prototype.clearModuleState = function() {};


  /**
   * Registers a callback to call once the dependency is actually requested
   * via goog.require + all of the immediate dependencies have been loaded or
   * all other files have been loaded. Allows for lazy loading until
   * require'd without pausing dependency loading, which is needed on old IE.
   *
   * @param {!Function} callback
   */
  goog.LoadController.prototype.defer = function(callback) {};


  /**
   * @return {boolean}
   */
  goog.LoadController.prototype.areDepsLoaded = function() {};


  /**
   * Basic super class for all dependencies Closure Library can load.
   *
   * This default implementation is designed to load untranspiled, non-module
   * scripts in a web broswer.
   *
   * For transpiled non-goog.module files {@see goog.TranspiledDependency}.
   * For goog.modules see {@see goog.GoogModuleDependency}.
   * For untranspiled ES6 modules {@see goog.Es6ModuleDependency}.
   *
   * @param {string} path Absolute path of this script.
   * @param {string} relativePath Path of this script relative to goog.basePath.
   * @param {!Array<string>} provides goog.provided or goog.module symbols
   *     in this file.
   * @param {!Array<string>} requires goog symbols or relative paths to Closure
   *     this depends on.
   * @param {!Object<string, string>} loadFlags
   * @struct @constructor
   */
  goog.Dependency = function(
      path, relativePath, provides, requires, loadFlags) {
    /** @const */
    this.path = path;
    /** @const */
    this.relativePath = relativePath;
    /** @const */
    this.provides = provides;
    /** @const */
    this.requires = requires;
    /** @const */
    this.loadFlags = loadFlags;
    /** @private {boolean} */
    this.loaded_ = false;
    /** @private {!Array<function()>} */
    this.loadCallbacks_ = [];
  };


  /**
   * @return {string} The pathname part of this dependency's path if it is a
   *     URI.
   */
  goog.Dependency.prototype.getPathName = function() {
    var pathName = this.path;
    var protocolIndex = pathName.indexOf('://');
    if (protocolIndex >= 0) {
      pathName = pathName.substring(protocolIndex + 3);
      var slashIndex = pathName.indexOf('/');
      if (slashIndex >= 0) {
        pathName = pathName.substring(slashIndex + 1);
      }
    }
    return pathName;
  };


  /**
   * @param {function()} callback Callback to fire as soon as this has loaded.
   * @final
   */
  goog.Dependency.prototype.onLoad = function(callback) {
    if (this.loaded_) {
      callback();
    } else {
      this.loadCallbacks_.push(callback);
    }
  };


  /**
   * Marks this dependency as loaded and fires any callbacks registered with
   * onLoad.
   * @final
   */
  goog.Dependency.prototype.loaded = function() {
    this.loaded_ = true;
    var callbacks = this.loadCallbacks_;
    this.loadCallbacks_ = [];
    for (var i = 0; i < callbacks.length; i++) {
      callbacks[i]();
    }
  };


  /**
   * Whether or not document.written / appended script tags should be deferred.
   *
   * @private {boolean}
   */
  goog.Dependency.defer_ = false;


  /**
   * Map of script ready / state change callbacks. Old IE cannot handle putting
   * these properties on goog.global.
   *
   * @private @const {!Object<string, function(?):undefined>}
   */
  goog.Dependency.callbackMap_ = {};


  /**
   * @param {function(...?):?} callback
   * @return {string}
   * @private
   */
  goog.Dependency.registerCallback_ = function(callback) {
    var key = Math.random().toString(32);
    goog.Dependency.callbackMap_[key] = callback;
    return key;
  };


  /**
   * @param {string} key
   * @private
   */
  goog.Dependency.unregisterCallback_ = function(key) {
    delete goog.Dependency.callbackMap_[key];
  };


  /**
   * @param {string} key
   * @param {...?} var_args
   * @private
   * @suppress {unusedPrivateMembers}
   */
  goog.Dependency.callback_ = function(key, var_args) {
    if (key in goog.Dependency.callbackMap_) {
      var callback = goog.Dependency.callbackMap_[key];
      var args = [];
      for (var i = 1; i < arguments.length; i++) {
        args.push(arguments[i]);
      }
      callback.apply(undefined, args);
    } else {
      var errorMessage = 'Callback key ' + key +
          ' does not exist (was base.js loaded more than once?).';
      // MOE:begin_strip
      // TODO(johnplaisted): Some people internally are mistakenly loading
      // base.js twice, and this can happen while a dependency is loading,
      // wiping out state.
      goog.logToConsole_(errorMessage);
      // MOE:end_strip
      // MOE:insert throw Error(errorMessage);
    }
  };


  /**
   * Starts loading this dependency. This dependency can pause loading if it
   * needs to and resume it later via the controller interface.
   *
   * When this is loaded it should call controller.loaded(). Note that this will
   * end up calling the loaded method of this dependency; there is no need to
   * call it explicitly.
   *
   * @param {!goog.LoadController} controller
   */
  goog.Dependency.prototype.load = function(controller) {
    if (goog.global.CLOSURE_IMPORT_SCRIPT) {
      if (goog.global.CLOSURE_IMPORT_SCRIPT(this.path)) {
        controller.loaded();
      } else {
        controller.pause();
      }
      return;
    }

    if (!goog.inHtmlDocument_()) {
      goog.logToConsole_(
          'Cannot use default debug loader outside of HTML documents.');
      if (this.relativePath == 'deps.js') {
        // Some old code is relying on base.js auto loading deps.js failing with
        // no error before later setting CLOSURE_IMPORT_SCRIPT.
        // CLOSURE_IMPORT_SCRIPT should be set *before* base.js is loaded, or
        // CLOSURE_NO_DEPS set to true.
        goog.logToConsole_(
            'Consider setting CLOSURE_IMPORT_SCRIPT before loading base.js, ' +
            'or setting CLOSURE_NO_DEPS to true.');
        controller.loaded();
      } else {
        controller.pause();
      }
      return;
    }

    /** @type {!HTMLDocument} */
    var doc = goog.global.document;

    // If the user tries to require a new symbol after document load,
    // something has gone terribly wrong. Doing a document.write would
    // wipe out the page. This does not apply to the CSP-compliant method
    // of writing script tags.
    if (doc.readyState == 'complete' &&
        !goog.ENABLE_CHROME_APP_SAFE_SCRIPT_LOADING) {
      // Certain test frameworks load base.js multiple times, which tries
      // to write deps.js each time. If that happens, just fail silently.
      // These frameworks wipe the page between each load of base.js, so this
      // is OK.
      var isDeps = /\bdeps.js$/.test(this.path);
      if (isDeps) {
        controller.loaded();
        return;
      } else {
        throw Error('Cannot write "' + this.path + '" after document load');
      }
    }

    var nonce = goog.getScriptNonce();
    if (!goog.ENABLE_CHROME_APP_SAFE_SCRIPT_LOADING &&
        goog.isDocumentLoading_()) {
      var key;
      var callback = function(script) {
        if (script.readyState && script.readyState != 'complete') {
          script.onload = callback;
          return;
        }
        goog.Dependency.unregisterCallback_(key);
        controller.loaded();
      };
      key = goog.Dependency.registerCallback_(callback);

      var defer = goog.Dependency.defer_ ? ' defer' : '';
      var nonceAttr = nonce ? ' nonce="' + nonce + '"' : '';
      var script = '<script src="' + this.path + '"' + nonceAttr + defer +
          ' id="script-' + key + '"><\/script>';

      script += '<script' + nonceAttr + '>';

      if (goog.Dependency.defer_) {
        script += 'document.getElementById(\'script-' + key +
            '\').onload = function() {\n' +
            '  goog.Dependency.callback_(\'' + key + '\', this);\n' +
            '};\n';
      } else {
        script += 'goog.Dependency.callback_(\'' + key +
            '\', document.getElementById(\'script-' + key + '\'));';
      }

      script += '<\/script>';

      doc.write(
          goog.TRUSTED_TYPES_POLICY_ ?
              goog.TRUSTED_TYPES_POLICY_.createHTML(script) :
              script);
    } else {
      var scriptEl =
          /** @type {!HTMLScriptElement} */ (doc.createElement('script'));
      scriptEl.defer = goog.Dependency.defer_;
      scriptEl.async = false;

      // If CSP nonces are used, propagate them to dynamically created scripts.
      // This is necessary to allow nonce-based CSPs without 'strict-dynamic'.
      if (nonce) {
        scriptEl.nonce = nonce;
      }

      if (goog.DebugLoader_.IS_OLD_IE_) {
        // Execution order is not guaranteed on old IE, halt loading and write
        // these scripts one at a time, after each loads.
        controller.pause();
        scriptEl.onreadystatechange = function() {
          if (scriptEl.readyState == 'loaded' ||
              scriptEl.readyState == 'complete') {
            controller.loaded();
            controller.resume();
          }
        };
      } else {
        scriptEl.onload = function() {
          scriptEl.onload = null;
          controller.loaded();
        };
      }

      scriptEl.src = goog.TRUSTED_TYPES_POLICY_ ?
          goog.TRUSTED_TYPES_POLICY_.createScriptURL(this.path) :
          this.path;
      doc.head.appendChild(scriptEl);
    }
  };


  /**
   * @param {string} path Absolute path of this script.
   * @param {string} relativePath Path of this script relative to goog.basePath.
   * @param {!Array<string>} provides Should be an empty array.
   *     TODO(johnplaisted) add support for adding closure namespaces to ES6
   *     modules for interop purposes.
   * @param {!Array<string>} requires goog symbols or relative paths to Closure
   *     this depends on.
   * @param {!Object<string, string>} loadFlags
   * @struct @constructor
   * @extends {goog.Dependency}
   */
  goog.Es6ModuleDependency = function(
      path, relativePath, provides, requires, loadFlags) {
    goog.Es6ModuleDependency.base(
        this, 'constructor', path, relativePath, provides, requires, loadFlags);
  };
  goog.inherits(goog.Es6ModuleDependency, goog.Dependency);


  /**
   * @override
   * @param {!goog.LoadController} controller
   */
  goog.Es6ModuleDependency.prototype.load = function(controller) {
    if (goog.global.CLOSURE_IMPORT_SCRIPT) {
      if (goog.global.CLOSURE_IMPORT_SCRIPT(this.path)) {
        controller.loaded();
      } else {
        controller.pause();
      }
      return;
    }

    if (!goog.inHtmlDocument_()) {
      goog.logToConsole_(
          'Cannot use default debug loader outside of HTML documents.');
      controller.pause();
      return;
    }

    /** @type {!HTMLDocument} */
    var doc = goog.global.document;

    var dep = this;

    // TODO(johnplaisted): Does document.writing really speed up anything? Any
    // difference between this and just waiting for interactive mode and then
    // appending?
    function write(src, contents) {
      var nonceAttr = '';
      var nonce = goog.getScriptNonce();
      if (nonce) {
        nonceAttr = ' nonce="' + nonce + '"';
      }

      if (contents) {
        var script = '<script type="module" crossorigin' + nonceAttr + '>' +
            contents + '</' +
            'script>';
        doc.write(
            goog.TRUSTED_TYPES_POLICY_ ?
                goog.TRUSTED_TYPES_POLICY_.createHTML(script) :
                script);
      } else {
        var script = '<script type="module" crossorigin src="' + src + '"' +
            nonceAttr + '></' +
            'script>';
        doc.write(
            goog.TRUSTED_TYPES_POLICY_ ?
                goog.TRUSTED_TYPES_POLICY_.createHTML(script) :
                script);
      }
    }

    function append(src, contents) {
      var scriptEl =
          /** @type {!HTMLScriptElement} */ (doc.createElement('script'));
      scriptEl.defer = true;
      scriptEl.async = false;
      scriptEl.type = 'module';
      scriptEl.setAttribute('crossorigin', true);

      // If CSP nonces are used, propagate them to dynamically created scripts.
      // This is necessary to allow nonce-based CSPs without 'strict-dynamic'.
      var nonce = goog.getScriptNonce();
      if (nonce) {
        scriptEl.nonce = nonce;
      }

      if (contents) {
        scriptEl.text = goog.TRUSTED_TYPES_POLICY_ ?
            goog.TRUSTED_TYPES_POLICY_.createScript(contents) :
            contents;
      } else {
        scriptEl.src = goog.TRUSTED_TYPES_POLICY_ ?
            goog.TRUSTED_TYPES_POLICY_.createScriptURL(src) :
            src;
      }

      doc.head.appendChild(scriptEl);
    }

    var create;

    if (goog.isDocumentLoading_()) {
      create = write;
      // We can ONLY call document.write if we are guaranteed that any
      // non-module script tags document.written after this are deferred.
      // Small optimization, in theory document.writing is faster.
      goog.Dependency.defer_ = true;
    } else {
      create = append;
    }

    // Write 4 separate tags here:
    // 1) Sets the module state at the correct time (just before execution).
    // 2) A src node for this, which just hopefully lets the browser load it a
    //    little early (no need to parse #3).
    // 3) Import the module and register it.
    // 4) Clear the module state at the correct time. Guaranteed to run even
    //    if there is an error in the module (#3 will not run if there is an
    //    error in the module).
    var beforeKey = goog.Dependency.registerCallback_(function() {
      goog.Dependency.unregisterCallback_(beforeKey);
      controller.setModuleState(goog.ModuleType.ES6);
    });
    create(undefined, 'goog.Dependency.callback_("' + beforeKey + '")');

    // TODO(johnplaisted): Does this really speed up anything?
    create(this.path, undefined);

    var registerKey = goog.Dependency.registerCallback_(function(exports) {
      goog.Dependency.unregisterCallback_(registerKey);
      controller.registerEs6ModuleExports(
          dep.path, exports, goog.moduleLoaderState_.moduleName);
    });
    create(
        undefined,
        'import * as m from "' + this.path + '"; goog.Dependency.callback_("' +
            registerKey + '", m)');

    var afterKey = goog.Dependency.registerCallback_(function() {
      goog.Dependency.unregisterCallback_(afterKey);
      controller.clearModuleState();
      controller.loaded();
    });
    create(undefined, 'goog.Dependency.callback_("' + afterKey + '")');
  };


  /**
   * Superclass of any dependency that needs to be loaded into memory,
   * transformed, and then eval'd (goog.modules and transpiled files).
   *
   * @param {string} path Absolute path of this script.
   * @param {string} relativePath Path of this script relative to goog.basePath.
   * @param {!Array<string>} provides goog.provided or goog.module symbols
   *     in this file.
   * @param {!Array<string>} requires goog symbols or relative paths to Closure
   *     this depends on.
   * @param {!Object<string, string>} loadFlags
   * @struct @constructor @abstract
   * @extends {goog.Dependency}
   */
  goog.TransformedDependency = function(
      path, relativePath, provides, requires, loadFlags) {
    goog.TransformedDependency.base(
        this, 'constructor', path, relativePath, provides, requires, loadFlags);
    /** @private {?string} */
    this.contents_ = null;

    /**
     * Whether to lazily make the synchronous XHR (when goog.require'd) or make
     * the synchronous XHR when initially loading. On FireFox 61 there is a bug
     * where an ES6 module cannot make a synchronous XHR (rather, it can, but if
     * it does then no other ES6 modules will load after).
     *
     * tl;dr we lazy load due to bugs on older browsers and eager load due to
     * bugs on newer ones.
     *
     * https://bugzilla.mozilla.org/show_bug.cgi?id=1477090
     *
     * @private @const {boolean}
     */
    this.lazyFetch_ = !goog.inHtmlDocument_() ||
        !('noModule' in goog.global.document.createElement('script'));
  };
  goog.inherits(goog.TransformedDependency, goog.Dependency);


  /**
   * @override
   * @param {!goog.LoadController} controller
   */
  goog.TransformedDependency.prototype.load = function(controller) {
    var dep = this;

    function fetch() {
      dep.contents_ = goog.loadFileSync_(dep.path);

      if (dep.contents_) {
        dep.contents_ = dep.transform(dep.contents_);
        if (dep.contents_) {
          dep.contents_ += '\n//# sourceURL=' + dep.path;
        }
      }
    }

    if (goog.global.CLOSURE_IMPORT_SCRIPT) {
      fetch();
      if (this.contents_ &&
          goog.global.CLOSURE_IMPORT_SCRIPT('', this.contents_)) {
        this.contents_ = null;
        controller.loaded();
      } else {
        controller.pause();
      }
      return;
    }


    var isEs6 = this.loadFlags['module'] == goog.ModuleType.ES6;

    if (!this.lazyFetch_) {
      fetch();
    }

    function load() {
      if (dep.lazyFetch_) {
        fetch();
      }

      if (!dep.contents_) {
        // loadFileSync_ or transform are responsible. Assume they logged an
        // error.
        return;
      }

      if (isEs6) {
        controller.setModuleState(goog.ModuleType.ES6);
      }

      var namespace;

      try {
        var contents = dep.contents_;
        dep.contents_ = null;
        goog.globalEval(goog.CLOSURE_EVAL_PREFILTER_.createScript(contents));
        if (isEs6) {
          namespace = goog.moduleLoaderState_.moduleName;
        }
      } finally {
        if (isEs6) {
          controller.clearModuleState();
        }
      }

      if (isEs6) {
        // Due to circular dependencies this may not be available for require
        // right now.
        goog.global['$jscomp']['require']['ensure'](
            [dep.getPathName()], function() {
              controller.registerEs6ModuleExports(
                  dep.path,
                  goog.global['$jscomp']['require'](dep.getPathName()),
                  namespace);
            });
      }

      controller.loaded();
    }

    // Do not fetch now; in FireFox 47 the synchronous XHR doesn't block all
    // events. If we fetched now and then document.write'd the contents the
    // document.write would be an eval and would execute too soon! Instead write
    // a script tag to fetch and eval synchronously at the correct time.
    function fetchInOwnScriptThenLoad() {
      /** @type {!HTMLDocument} */
      var doc = goog.global.document;

      var key = goog.Dependency.registerCallback_(function() {
        goog.Dependency.unregisterCallback_(key);
        load();
      });

      var nonce = goog.getScriptNonce();
      var nonceAttr = nonce ? ' nonce="' + nonce + '"' : '';
      var script = '<script' + nonceAttr + '>' +
          goog.protectScriptTag_('goog.Dependency.callback_("' + key + '");') +
          '</' +
          'script>';
      doc.write(
          goog.TRUSTED_TYPES_POLICY_ ?
              goog.TRUSTED_TYPES_POLICY_.createHTML(script) :
              script);
    }

    // If one thing is pending it is this.
    var anythingElsePending = controller.pending().length > 1;

    // If anything else is loading we need to lazy load due to bugs in old IE.
    // Specifically script tags with src and script tags with contents could
    // execute out of order if document.write is used, so we cannot use
    // document.write. Do not pause here; it breaks old IE as well.
    var useOldIeWorkAround =
        anythingElsePending && goog.DebugLoader_.IS_OLD_IE_;

    // Additionally if we are meant to defer scripts but the page is still
    // loading (e.g. an ES6 module is loading) then also defer. Or if we are
    // meant to defer and anything else is pending then defer (those may be
    // scripts that did not need transformation and are just script tags with
    // defer set to true, and we need to evaluate after that deferred script).
    var needsAsyncLoading = goog.Dependency.defer_ &&
        (anythingElsePending || goog.isDocumentLoading_());

    if (useOldIeWorkAround || needsAsyncLoading) {
      // Note that we only defer when we have to rather than 100% of the time.
      // Always defering would work, but then in theory the order of
      // goog.require calls would then matter. We want to enforce that most of
      // the time the order of the require calls does not matter.
      controller.defer(function() {
        load();
      });
      return;
    }
    // TODO(johnplaisted): Externs are missing onreadystatechange for
    // HTMLDocument.
    /** @type {?} */
    var doc = goog.global.document;

    var isInternetExplorerOrEdge = goog.inHtmlDocument_() &&
        ('ActiveXObject' in goog.global || goog.isEdge_());

    // Don't delay in any version of IE or pre-Chromium Edge. There's a bug
    // around this that will cause out of order script execution. This means
    // that on older IE ES6 modules will load too early (while the document is
    // still loading + the dom is not available). The other option is to load
    // too late (when the document is complete and the onload even will never
    // fire). This seems to be the lesser of two evils as scripts already act
    // like the former.
    if (isEs6 && goog.inHtmlDocument_() && goog.isDocumentLoading_() &&
        !isInternetExplorerOrEdge) {
      goog.Dependency.defer_ = true;
      // Transpiled ES6 modules still need to load like regular ES6 modules,
      // aka only after the document is interactive.
      controller.pause();
      var oldCallback = doc.onreadystatechange;
      doc.onreadystatechange = function() {
        if (doc.readyState == 'interactive') {
          doc.onreadystatechange = oldCallback;
          load();
          controller.resume();
        }
        if (typeof oldCallback === 'function') {
          oldCallback.apply(undefined, arguments);
        }
      };
    } else {
      // Always eval on old IE.
      if (goog.DebugLoader_.IS_OLD_IE_ || !goog.inHtmlDocument_() ||
          !goog.isDocumentLoading_()) {
        load();
      } else {
        fetchInOwnScriptThenLoad();
      }
    }
  };


  /**
   * @param {string} contents
   * @return {string}
   * @abstract
   */
  goog.TransformedDependency.prototype.transform = function(contents) {};


  /**
   * Any non-goog.module dependency which needs to be transpiled before eval.
   *
   * @param {string} path Absolute path of this script.
   * @param {string} relativePath Path of this script relative to goog.basePath.
   * @param {!Array<string>} provides goog.provided or goog.module symbols
   *     in this file.
   * @param {!Array<string>} requires goog symbols or relative paths to Closure
   *     this depends on.
   * @param {!Object<string, string>} loadFlags
   * @param {!goog.Transpiler} transpiler
   * @struct @constructor
   * @extends {goog.TransformedDependency}
   */
  goog.TranspiledDependency = function(
      path, relativePath, provides, requires, loadFlags, transpiler) {
    goog.TranspiledDependency.base(
        this, 'constructor', path, relativePath, provides, requires, loadFlags);
    /** @protected @const*/
    this.transpiler = transpiler;
  };
  goog.inherits(goog.TranspiledDependency, goog.TransformedDependency);


  /**
   * @override
   * @param {string} contents
   * @return {string}
   */
  goog.TranspiledDependency.prototype.transform = function(contents) {
    // Transpile with the pathname so that ES6 modules are domain agnostic.
    return this.transpiler.transpile(contents, this.getPathName());
  };


  /**
   * An ES6 module dependency that was transpiled to a jscomp module outside
   * of the debug loader, e.g. server side.
   *
   * @param {string} path Absolute path of this script.
   * @param {string} relativePath Path of this script relative to goog.basePath.
   * @param {!Array<string>} provides goog.provided or goog.module symbols
   *     in this file.
   * @param {!Array<string>} requires goog symbols or relative paths to Closure
   *     this depends on.
   * @param {!Object<string, string>} loadFlags
   * @struct @constructor
   * @extends {goog.TransformedDependency}
   */
  goog.PreTranspiledEs6ModuleDependency = function(
      path, relativePath, provides, requires, loadFlags) {
    goog.PreTranspiledEs6ModuleDependency.base(
        this, 'constructor', path, relativePath, provides, requires, loadFlags);
  };
  goog.inherits(
      goog.PreTranspiledEs6ModuleDependency, goog.TransformedDependency);


  /**
   * @override
   * @param {string} contents
   * @return {string}
   */
  goog.PreTranspiledEs6ModuleDependency.prototype.transform = function(
      contents) {
    return contents;
  };


  /**
   * A goog.module, transpiled or not. Will always perform some minimal
   * transformation even when not transpiled to wrap in a goog.loadModule
   * statement.
   *
   * @param {string} path Absolute path of this script.
   * @param {string} relativePath Path of this script relative to goog.basePath.
   * @param {!Array<string>} provides goog.provided or goog.module symbols
   *     in this file.
   * @param {!Array<string>} requires goog symbols or relative paths to Closure
   *     this depends on.
   * @param {!Object<string, string>} loadFlags
   * @param {boolean} needsTranspile
   * @param {!goog.Transpiler} transpiler
   * @struct @constructor
   * @extends {goog.TransformedDependency}
   */
  goog.GoogModuleDependency = function(
      path, relativePath, provides, requires, loadFlags, needsTranspile,
      transpiler) {
    goog.GoogModuleDependency.base(
        this, 'constructor', path, relativePath, provides, requires, loadFlags);
    /** @private @const */
    this.needsTranspile_ = needsTranspile;
    /** @private @const */
    this.transpiler_ = transpiler;
  };
  goog.inherits(goog.GoogModuleDependency, goog.TransformedDependency);


  /**
   * @override
   * @param {string} contents
   * @return {string}
   */
  goog.GoogModuleDependency.prototype.transform = function(contents) {
    if (this.needsTranspile_) {
      contents = this.transpiler_.transpile(contents, this.getPathName());
    }

    if (!goog.LOAD_MODULE_USING_EVAL || goog.global.JSON === undefined) {
      return '' +
          'goog.loadModule(function(exports) {' +
          '"use strict";' + contents +
          '\n' +  // terminate any trailing single line comment.
          ';return exports' +
          '});' +
          '\n//# sourceURL=' + this.path + '\n';
    } else {
      return '' +
          'goog.loadModule(' +
          goog.global.JSON.stringify(
              contents + '\n//# sourceURL=' + this.path + '\n') +
          ');';
    }
  };


  /**
   * Whether the browser is IE9 or earlier, which needs special handling
   * for deferred modules.
   * @const @private {boolean}
   */
  goog.DebugLoader_.IS_OLD_IE_ = !!(
      !goog.global.atob && goog.global.document && goog.global.document['all']);


  /**
   * @param {string} relPath
   * @param {!Array<string>|undefined} provides
   * @param {!Array<string>} requires
   * @param {boolean|!Object<string>=} opt_loadFlags
   * @see goog.addDependency
   */
  goog.DebugLoader_.prototype.addDependency = function(
      relPath, provides, requires, opt_loadFlags) {
    provides = provides || [];
    relPath = relPath.replace(/\\/g, '/');
    var path = goog.normalizePath_(goog.basePath + relPath);
    if (!opt_loadFlags || typeof opt_loadFlags === 'boolean') {
      opt_loadFlags = opt_loadFlags ? {'module': goog.ModuleType.GOOG} : {};
    }
    var dep = this.factory_.createDependency(
        path, relPath, provides, requires, opt_loadFlags,
        goog.transpiler_.needsTranspile(
            opt_loadFlags['lang'] || 'es3', opt_loadFlags['module']));
    this.dependencies_[path] = dep;
    for (var i = 0; i < provides.length; i++) {
      this.idToPath_[provides[i]] = path;
    }
    this.idToPath_[relPath] = path;
  };


  /**
   * Creates goog.Dependency instances for the debug loader to load.
   *
   * Should be overridden to have the debug loader use custom subclasses of
   * goog.Dependency.
   *
   * @param {!goog.Transpiler} transpiler
   * @struct @constructor
   */
  goog.DependencyFactory = function(transpiler) {
    /** @protected @const */
    this.transpiler = transpiler;
  };


  /**
   * @param {string} path Absolute path of the file.
   * @param {string} relativePath Path relative to closure’s base.js.
   * @param {!Array<string>} provides Array of provided goog.provide/module ids.
   * @param {!Array<string>} requires Array of required goog.provide/module /
   *     relative ES6 module paths.
   * @param {!Object<string, string>} loadFlags
   * @param {boolean} needsTranspile True if the file needs to be transpiled
   *     per the goog.Transpiler.
   * @return {!goog.Dependency}
   */
  goog.DependencyFactory.prototype.createDependency = function(
      path, relativePath, provides, requires, loadFlags, needsTranspile) {
    // MOE:begin_strip
    var provide, require;
    for (var i = 0; provide = provides[i]; i++) {
      goog.dependencies_.nameToPath[provide] = relativePath;
      goog.dependencies_.loadFlags[relativePath] = loadFlags;
    }
    for (var j = 0; require = requires[j]; j++) {
      if (!(relativePath in goog.dependencies_.requires)) {
        goog.dependencies_.requires[relativePath] = {};
      }
      goog.dependencies_.requires[relativePath][require] = true;
    }
    // MOE:end_strip

    if (loadFlags['module'] == goog.ModuleType.GOOG) {
      return new goog.GoogModuleDependency(
          path, relativePath, provides, requires, loadFlags, needsTranspile,
          this.transpiler);
    } else if (needsTranspile) {
      return new goog.TranspiledDependency(
          path, relativePath, provides, requires, loadFlags, this.transpiler);
    } else {
      if (loadFlags['module'] == goog.ModuleType.ES6) {
        if (goog.TRANSPILE == 'never' && goog.ASSUME_ES_MODULES_TRANSPILED) {
          return new goog.PreTranspiledEs6ModuleDependency(
              path, relativePath, provides, requires, loadFlags);
        } else {
          return new goog.Es6ModuleDependency(
              path, relativePath, provides, requires, loadFlags);
        }
      } else {
        return new goog.Dependency(
            path, relativePath, provides, requires, loadFlags);
      }
    }
  };


  /** @private @const */
  goog.debugLoader_ = new goog.DebugLoader_();


  /**
   * Loads the Closure Dependency file.
   *
   * Exposed a public function so CLOSURE_NO_DEPS can be set to false, base
   * loaded, setDependencyFactory called, and then this called. i.e. allows
   * custom loading of the deps file.
   */
  goog.loadClosureDeps = function() {
    goog.debugLoader_.loadClosureDeps();
  };


  /**
   * Sets the dependency factory, which can be used to create custom
   * goog.Dependency implementations to control how dependencies are loaded.
   *
   * Note: if you wish to call this function and provide your own implemnetation
   * it is a wise idea to set CLOSURE_NO_DEPS to true, otherwise the dependency
   * file and all of its goog.addDependency calls will use the default factory.
   * You can call goog.loadClosureDeps to load the Closure dependency file
   * later, after your factory is injected.
   *
   * @param {!goog.DependencyFactory} factory
   */
  goog.setDependencyFactory = function(factory) {
    goog.debugLoader_.setDependencyFactory(factory);
  };


  /**
   * Trusted Types policy for the debug loader.
   * @private @const {?TrustedTypePolicy}
   */
  goog.TRUSTED_TYPES_POLICY_ = goog.TRUSTED_TYPES_POLICY_NAME ?
      goog.createTrustedTypesPolicy(goog.TRUSTED_TYPES_POLICY_NAME + '#base') :
      null;

  if (!goog.global.CLOSURE_NO_DEPS) {
    goog.debugLoader_.loadClosureDeps();
  }


  /**
   * Bootstraps the given namespaces and calls the callback once they are
   * available either via goog.require. This is a replacement for using
   * `goog.require` to bootstrap Closure JavaScript. Previously a `goog.require`
   * in an HTML file would guarantee that the require'd namespace was available
   * in the next immediate script tag. With ES6 modules this no longer a
   * guarantee.
   *
   * @param {!Array<string>} namespaces
   * @param {function(): ?} callback Function to call once all the namespaces
   *     have loaded. Always called asynchronously.
   */
  goog.bootstrap = function(namespaces, callback) {
    goog.debugLoader_.bootstrap(namespaces, callback);
  };
}


if (!COMPILED) {
  var isChrome87 = false;
  // Cannot run check for Chrome <87 bug in case of strict CSP environments.
  // TODO(aaronshim): Remove once Chrome <87 bug is no longer a problem.
  try {
    isChrome87 = eval(goog.global.trustedTypes.emptyScript) !==
        goog.global.trustedTypes.emptyScript;
  } catch (err) {
  }

  /**
   * Trusted Types for running dev servers.
   *
   * @private @const
   */
  goog.CLOSURE_EVAL_PREFILTER_ =
      // Detect Chrome <87 bug with TT and eval.
      goog.global.trustedTypes && isChrome87 &&
          goog.createTrustedTypesPolicy('goog#base#devonly#eval') ||
      {createScript: goog.identity_};
}

//third_party/javascript/tslib/tslib_closure.js
goog.loadModule(function(exports) {'use strict';/**
 * @fileoverview
 * Hand-modified Closure version of tslib.js.
 * These use the literal space optimized code from TypeScript for
 * compatibility.
 *
 * @suppress {undefinedVars}
 */

// Do not use @license

/******************************************************************************
Copyright (c) Microsoft Corporation.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
***************************************************************************** */

goog.module('google3.third_party.javascript.tslib.tslib');

/** @suppress {missingPolyfill} the code below intentionally feature-tests. */
var extendStatics = Object.setPrototypeOf ||
    ({__proto__: []} instanceof Array && function(d, b) {d.__proto__ = b;}) ||
    function(d, b) { for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p]; };

/**
 * @param {?} d
 * @param {?} b
 */
exports.__extends = function(d, b) {
    extendStatics(d, b);
    // LOCAL MODIFICATION: Add jsdoc annotation here:
    /** @constructor */
    function __() { /** @type {?} */ (this).constructor = d; }
    d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
};

exports.__assign = Object.assign || /** @return {?} */ function (/** ? */ t) {
    for (var s, i = 1, n = arguments.length; i < n; i++) {
        s = arguments[i];
        for (var p in s) if (Object.prototype.hasOwnProperty.call(s, p)) t[p] = s[p];
    }
    return t;
};

/**
 * @param {?} s
 * @param {?} e
 * @return {?}
 */
exports.__rest = function (s, e) {
    var t = {};
    for (var p in s) if (Object.prototype.hasOwnProperty.call(s, p) && e.indexOf(p) < 0)
        t[p] = s[p];
    if (s != null && typeof Object.getOwnPropertySymbols === "function")
        for (var i = 0, p = Object.getOwnPropertySymbols(s); i < p.length; i++) if (e.indexOf(p[i]) < 0)
            t[p[i]] = s[p[i]];
    return t;
};

/**
 * @param {?} decorators
 * @param {T} target
 * @param {?=} key
 * @param {?=} desc
 * @return {T}
 * @template T
 */
exports.__decorate = function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    // google3 local modification: use quoted property access to work around
    // https://b.corp.google.com/issues/77140019.
    if (typeof Reflect === "object" && Reflect && typeof Reflect['decorate'] === "function") r = Reflect['decorate'](decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};

/**
 * @param {?} metadataKey
 * @param {?} metadataValue
 * @return {?}
 */
exports.__metadata = function (metadataKey, metadataValue) {
  // google3 local modification: use quoted property access to work around
  // https://b.corp.google.com/issues/77140019.
  if (typeof Reflect === "object" && Reflect && typeof Reflect['metadata'] === "function") return Reflect['metadata'](metadataKey, metadataValue);
};

/**
 * @param {?} paramIndex
 * @param {?} decorator
 * @return {?}
 */
exports.__param = function (paramIndex, decorator) {
    return function (target, key) { decorator(target, key, paramIndex); };
};

/**
 * @template T
 * @param {T} thisArg
 * @param {?} _arguments
 * @param {?} P
 * @param {function(this:T)} generator
 * @return {?}
 */
exports.__awaiter = function(thisArg, _arguments, P, generator) {
  return new (P || (P = Promise))(function(resolve, reject) {
    // LOCAL MODIFICATION: Cannot express the function + keys pattern in
    // closure, so we escape generator.next with ? type.
    function fulfilled(value) {
      try {
        step((/** @type {?} */ (generator)).next(value));
      } catch (e) {
        reject(e);
      }
    }
    function rejected(value) {
      try {
        step(generator['throw'](value));
      } catch (e) {
        reject(e);
      }
    }
    function step(result) {
      result.done ? resolve(result.value) : new P(function(resolve) {
                                              resolve(result.value);
                                            }).then(fulfilled, rejected);
    }
    step((generator = generator.apply(thisArg, _arguments)).next());
  });
};

/**
 * @param {?} thisArg
 * @param {?} body
 * @return {?}
 */
exports.__generator = function(thisArg, body) {
  var _ = {
    label: 0,
    sent: function() {
      if (t[0] & 1) throw (/** @type {!Error} */ (t[1]));
      return t[1];
    },
    trys: [],
    ops: []
  },
      f, y, t, g;
  // LOCAL MODIFICATION: Originally iterator body was "return this", but it
  // doesn't compile as this is unknown. Changed to g, which is equivalent.
  return g = {next: verb(0), "throw": verb(1), "return": verb(2)},
         typeof Symbol === "function" && (g[Symbol.iterator] = function() {
           return g;
         }), g;
  function verb(n) {
    return function(v) {
      return step([n, v]);
    };
  }
  function step(op) {
    if (f) throw new TypeError("Generator is already executing.");
    while (_) try {
        if (f = 1,
            y && (t = y[op[0] & 2 ? "return" : op[0] ? "throw" : "next"]) &&
                !(t = t.call(y, op[1])).done)
          return t;
        if (y = 0, t) op = [0, t.value];
        switch (op[0]) {
          case 0:
          case 1:
            t = op;
            break;
          case 4:
            _.label++;
            return {value: op[1], done: false};
          case 5:
            _.label++;
            y = op[1];
            op = [0];
            continue;
          case 7:
            op = _.ops.pop();
            _.trys.pop();
            continue;
          default:
            if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) &&
                (op[0] === 6 || op[0] === 2)) {
              _ = 0;
              continue;
            }
            if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) {
              _.label = op[1];
              break;
            }
            if (op[0] === 6 && _.label < t[1]) {
              _.label = t[1];
              t = op;
              break;
            }
            if (t && _.label < t[2]) {
              _.label = t[2];
              _.ops.push(op);
              break;
            }
            if (t[2]) _.ops.pop();
            _.trys.pop();
            continue;
        }
        op = body.call(thisArg, _);
      } catch (e) {
        op = [6, e];
        y = 0;
      } finally {
        f = t = 0;
      }
    if (op[0] & 5) throw (/** @type {!Error} */ (op[1]));
    return {value: op[0] ? op[1] : void 0, done: true};
  }
};

/**
 * @param {?} m
 * @param {?} e
 */
exports.__exportStar = function (m, e) {
    for (var p in m) if (!e.hasOwnProperty(p)) e[p] = m[p];
};

/**
 * @param {?} o
 * @return {?}
 */
exports.__values = function (o) {
    var m = typeof Symbol === "function" && o[Symbol.iterator], i = 0;
    if (m) return m.call(o);
    return {
        next: function () {
            if (o && i >= o.length) o = void 0;
            return { value: o && o[i++], done: !o };
        }
    };
};

/**
 * @param {?} o
 * @param {?=} n
 * @return {?}
 */
exports.__read = function(o, n) {
  var m = typeof Symbol === "function" && o[Symbol.iterator];
  if (!m) return o;
  var i = m.call(o), r, ar = [], e;
  try {
    while ((n === void 0 || n-- > 0) && !(r = i.next()).done) ar.push(r.value);
  } catch (error) {
    e = {error: error};
  } finally {
    try {
      if (r && !r.done && (m = i["return"])) m.call(i);
    } finally {
      if (e) throw (/** @type {!Error} */ (e.error));
    }
  }
  return ar;
};

/**
 * @return {!Array}
 * @deprecated since TypeScript 4.2
 */
exports.__spread = function() {
  for (var ar = [], i = 0; i < arguments.length; i++)
    ar = ar.concat(exports.__read(arguments[i]));
  return ar;
};

/**
 * @return {!Array<?>}
 * @deprecated since TypeScript 4.2
 */
exports.__spreadArrays = function() {
  for (var s = 0, i = 0, il = arguments.length; i < il; i++)
    s += arguments[i].length;
  for (var r = Array(s), k = 0, i = 0; i < il; i++)
    for (var a = arguments[i], j = 0, jl = a.length; j < jl; j++, k++)
      r[k] = a[j];
  return r;
};

/**
 * @param {!Array<?>} to
 * @param {!Array<?>} from
 * @return {!Array<?>}
 */
exports.__spreadArray = function(to, from) {
  // LOCAL MODIFICATION: https://github.com/microsoft/TypeScript/issues/43353
  // We have to accept NodeList because they don't implement Iterable in Edge.
  if (!Array.isArray(from) && !(from instanceof NodeList)) {
    throw new TypeError('Expected an Array or NodeList: ' + String(from));
  }
  // END LOCAL MODIFICATION
  for (var i = 0, il = from.length, j = to.length; i < il; i++, j++)
    to[j] = from[i];
  return to;
};

/**
 * @constructor
 * LOCAL MODIFICATION: Originally used "this" in function body,
 * @this {?}
 * END LOCAL MODIFICATION
 * @param {?} v
 * @return {?}
 */
exports.__await = function(v) {
  return this instanceof exports.__await ? (this.v = v, this) :
                                           new exports.__await(v);
};

/**
 * @template T
 * @param {T} thisArg
 * @param {?} _arguments
 * @param {function(this:T)} generator
 * @return {?}
 */
exports.__asyncGenerator = function __asyncGenerator(
    thisArg, _arguments, generator) {
  if (!Symbol.asyncIterator)
    throw new TypeError('Symbol.asyncIterator is not defined.');
  var g = generator.apply(thisArg, _arguments || []), i, q = [];
  return i = {}, verb('next'), verb('throw'), verb('return'),
         i[Symbol.asyncIterator] = function() {
           return (/** @type {?} */ (this));
         }, i;
  function verb(n) {
    if (g[n])
      i[n] = function(v) {
        return new Promise(function(a, b) {
          q.push([n, v, a, b]) > 1 || resume(n, v);
        });
      };
  }
  function resume(n, v) {
    try {
      step(g[n](v));
    } catch (e) {
      settle(q[0][3], e);
    }
  }
  function step(r) {
    r.value instanceof exports.__await ?
        Promise.resolve(/** @type {?} */ (r.value).v).then(fulfill, reject) :
        settle(q[0][2], r);
  }
  function fulfill(value) {
    resume('next', value);
  }
  function reject(value) {
    resume('throw', value);
  }
  function settle(f, v) {
    if (f(v), q.shift(), q.length) resume(q[0][0], q[0][1]);
  }
};

/**
 * @param {?} o
 * @return {?}
 */
exports.__asyncDelegator = function(o) {
  var i, p;
  // LOCAL MODIFICATION: Originally iterator body was "return this", but it
  // doesn't compile in some builds, as this is unknown. Changed to i, which is
  // equivalent.
  return i = {}, verb("next"), verb("throw", function (e) { throw e; }), verb("return"), i[Symbol.iterator] = function () { return i; }, i;
  /**
   * @param {?} n
   * @param {?=} f
   * @return {?}
   */
  function verb(n, f) { if (o[n]) i[n] = function (v) { return (p = !p) ? { value: new exports.__await(o[n](v)), done: n === "return" } : f ? f(v) : v; }; }
};

/**
 * @param {?} o
 * @return {?}
 */
exports.__asyncValues = function(o) {
  if (!Symbol.asyncIterator) throw new TypeError("Symbol.asyncIterator is not defined.");
  var m = o[Symbol.asyncIterator];
  return m ? m.call(o) : typeof __values === "function" ? __values(o) : o[Symbol.iterator]();
};

/**
 * @param {?=} cooked
 * @param {?=} raw
 * @return {?}
 */
exports.__makeTemplateObject = function(cooked, raw) {
    if (Object.defineProperty) { Object.defineProperty(cooked, "raw", { value: raw }); } else { cooked.raw = raw; }
    return cooked;
};


/**
 * @param {?} receiver
 * @param {!WeakMap} privateMap
 * @return {?}
 */
exports.__classPrivateFieldGet = function (receiver, privateMap) {
  if (!privateMap.has(receiver)) {
      throw new TypeError("attempted to get private field on non-instance");
  }
  return privateMap.get(receiver);
};

/**
 * @param {?} receiver
 * @param {!WeakMap} privateMap
 * @param {?} value
 * @return {?}
 */
exports.__classPrivateFieldSet = function (receiver, privateMap, value) {
  if (!privateMap.has(receiver)) {
      throw new TypeError("attempted to set private field on non-instance");
  }
  privateMap.set(receiver, value);
  return value;
};

;return exports;});

//third_party/mediapipe/web/solutions/shader_utils.closure.js
goog.loadModule(function(exports) {'use strict';/**
 *
 * @fileoverview Wrappers around the OpenGL shader creation calls.
 *
 * Generated from: third_party/mediapipe/web/solutions/shader_utils.ts
 * @suppress {checkTypes,extraRequire,missingOverride,missingRequire,missingReturn,unusedPrivateMembers,uselessCode} checked by tsc
 */
goog.module('google3.third_party.mediapipe.web.solutions.shader_utils');
var module = module || { id: 'third_party/mediapipe/web/solutions/shader_utils.closure.js' };
goog.require('google3.third_party.javascript.tslib.tslib');
/** @enum {number} */
const ShaderType = {
    VERTEX_SHADER: 0,
    FRAGMENT_SHADER: 1,
};
exports.ShaderType = ShaderType;
ShaderType[ShaderType.VERTEX_SHADER] = 'VERTEX_SHADER';
ShaderType[ShaderType.FRAGMENT_SHADER] = 'FRAGMENT_SHADER';
/**
 * Compiles a string into the requested shader type. We can always just use
 * `createProgramFromSources` to avoid calling this directly.
 * @param {!WebGLRenderingContext} gl
 * @param {string} source
 * @param {!ShaderType} type
 * @return {!WebGLShader}
 */
function loadShader(gl, source, type) {
    /** @type {number} */
    const shaderType = type === ShaderType.VERTEX_SHADER ? gl.VERTEX_SHADER : gl.FRAGMENT_SHADER;
    /** @type {!WebGLShader} */
    const shader = (/** @type {!WebGLShader} */ (gl.createShader(shaderType)));
    gl.shaderSource(shader, source);
    gl.compileShader(shader);
    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        /** @type {(null|string)} */
        const info = gl.getShaderInfoLog(shader);
        throw new Error(`Could not compile WebGL shader.\n\n${info}`);
    }
    return shader;
}
exports.loadShader = loadShader;
/**
 * Links a shader program given the vertex and fragment shaders.
 * @param {!WebGLRenderingContext} gl
 * @param {!WebGLShader} vertexShader
 * @param {!WebGLShader} fragmentShader
 * @return {!WebGLProgram}
 */
function createProgram(gl, vertexShader, fragmentShader) {
    /** @type {!WebGLProgram} */
    const program = (/** @type {!WebGLProgram} */ (gl.createProgram()));
    gl.attachShader(program, vertexShader);
    gl.attachShader(program, fragmentShader);
    gl.linkProgram(program);
    if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
        /** @type {(null|string)} */
        const info = gl.getProgramInfoLog(program);
        throw new Error(`Could not compile WebGL program.\n\n${info}`);
    }
    return program;
}
exports.createProgram = createProgram;
/**
 * Helper function to take shader programs as strings and produce a linked
 * program.
 * @param {!WebGLRenderingContext} gl
 * @param {string} vertexSource
 * @param {string} fragmentSource
 * @return {!WebGLProgram}
 */
function createProgramFromSources(gl, vertexSource, fragmentSource) {
    /** @type {!WebGLShader} */
    const vertexShader = loadShader(gl, vertexSource, ShaderType.VERTEX_SHADER);
    /** @type {!WebGLShader} */
    const fragmentShader = loadShader(gl, fragmentSource, ShaderType.FRAGMENT_SHADER);
    return createProgram(gl, vertexShader, fragmentShader);
}
exports.createProgramFromSources = createProgramFromSources;
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoic2hhZGVyX3V0aWxzLmpzIiwic291cmNlUm9vdCI6IiIsInNvdXJjZXMiOlsiLi4vLi4vLi4vLi4vLi4vLi4vLi4vdGhpcmRfcGFydHkvbWVkaWFwaXBlL3dlYi9zb2x1dGlvbnMvc2hhZGVyX3V0aWxzLnRzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiI7Ozs7Ozs7Ozs7O0FBUUEsTUFBWSxVQUFVO0lBQ3BCLGFBQWEsR0FBQTtJQUNiLGVBQWUsR0FBQTtFQUNoQjs7Ozs7Ozs7Ozs7O0FBTUQsU0FBZ0IsVUFBVSxDQUN0QixFQUF5QixFQUFFLE1BQWMsRUFBRSxJQUFnQjs7VUFDdkQsVUFBVSxHQUNaLElBQUksS0FBSyxVQUFVLENBQUMsYUFBYSxDQUFDLENBQUMsQ0FBQyxFQUFFLENBQUMsYUFBYSxDQUFDLENBQUMsQ0FBQyxFQUFFLENBQUMsZUFBZTs7VUFDdkUsTUFBTSxHQUFHLDhCQUFBLEVBQUUsQ0FBQyxZQUFZLENBQUMsVUFBVSxDQUFDLEVBQUM7SUFDM0MsRUFBRSxDQUFDLFlBQVksQ0FBQyxNQUFNLEVBQUUsTUFBTSxDQUFDLENBQUM7SUFDaEMsRUFBRSxDQUFDLGFBQWEsQ0FBQyxNQUFNLENBQUMsQ0FBQztJQUN6QixJQUFJLENBQUMsRUFBRSxDQUFDLGtCQUFrQixDQUFDLE1BQU0sRUFBRSxFQUFFLENBQUMsY0FBYyxDQUFDLEVBQUU7O2NBQy9DLElBQUksR0FBRyxFQUFFLENBQUMsZ0JBQWdCLENBQUMsTUFBTSxDQUFDO1FBQ3hDLE1BQU0sSUFBSSxLQUFLLENBQUMsc0NBQXNDLElBQUksRUFBRSxDQUFDLENBQUM7S0FDL0Q7SUFDRCxPQUFPLE1BQU0sQ0FBQztBQUNoQixDQUFDO0FBWkQsZ0NBWUM7Ozs7Ozs7O0FBS0QsU0FBZ0IsYUFBYSxDQUN6QixFQUF5QixFQUFFLFlBQXlCLEVBQ3BELGNBQTJCOztVQUN2QixPQUFPLEdBQUcsK0JBQUEsRUFBRSxDQUFDLGFBQWEsRUFBRSxFQUFDO0lBQ25DLEVBQUUsQ0FBQyxZQUFZLENBQUMsT0FBTyxFQUFFLFlBQVksQ0FBQyxDQUFDO0lBQ3ZDLEVBQUUsQ0FBQyxZQUFZLENBQUMsT0FBTyxFQUFFLGNBQWMsQ0FBQyxDQUFDO0lBQ3pDLEVBQUUsQ0FBQyxXQUFXLENBQUMsT0FBTyxDQUFDLENBQUM7SUFDeEIsSUFBSSxDQUFDLEVBQUUsQ0FBQyxtQkFBbUIsQ0FBQyxPQUFPLEVBQUUsRUFBRSxDQUFDLFdBQVcsQ0FBQyxFQUFFOztjQUM5QyxJQUFJLEdBQUcsRUFBRSxDQUFDLGlCQUFpQixDQUFDLE9BQU8sQ0FBQztRQUMxQyxNQUFNLElBQUksS0FBSyxDQUFDLHVDQUF1QyxJQUFJLEVBQUUsQ0FBQyxDQUFDO0tBQ2hFO0lBQ0QsT0FBTyxPQUFPLENBQUM7QUFDakIsQ0FBQztBQVpELHNDQVlDOzs7Ozs7Ozs7QUFNRCxTQUFnQix3QkFBd0IsQ0FDcEMsRUFBeUIsRUFBRSxZQUFvQixFQUMvQyxjQUFzQjs7VUFDbEIsWUFBWSxHQUFHLFVBQVUsQ0FBQyxFQUFFLEVBQUUsWUFBWSxFQUFFLFVBQVUsQ0FBQyxhQUFhLENBQUM7O1VBQ3JFLGNBQWMsR0FDaEIsVUFBVSxDQUFDLEVBQUUsRUFBRSxjQUFjLEVBQUUsVUFBVSxDQUFDLGVBQWUsQ0FBQztJQUM5RCxPQUFPLGFBQWEsQ0FBQyxFQUFFLEVBQUUsWUFBWSxFQUFFLGNBQWMsQ0FBQyxDQUFDO0FBQ3pELENBQUM7QUFQRCw0REFPQyIsInNvdXJjZXNDb250ZW50IjpbIi8qKlxuICogQGZpbGVvdmVydmlldyBXcmFwcGVycyBhcm91bmQgdGhlIE9wZW5HTCBzaGFkZXIgY3JlYXRpb24gY2FsbHMuXG4gKi9cblxuLyoqXG4gKiBTaW1wbGUgcmVmZXJlbmNlIHRvIHRoZSB0d28gc2hhZGVyIHR5cGVzLiBJZiB3ZSBkb24ndCBwcm92aWRlIHRoaXMsIHRoZVxuICogdXNlciBuZWVkcyB0byByZWFjaCBpbnRvIHRoZWlyIGdsQ29udGV4dCB0byBnZXQgdGhlIHZhbHVlcy5cbiAqL1xuZXhwb3J0IGVudW0gU2hhZGVyVHlwZSB7XG4gIFZFUlRFWF9TSEFERVIsXG4gIEZSQUdNRU5UX1NIQURFUlxufVxuXG4vKipcbiAqIENvbXBpbGVzIGEgc3RyaW5nIGludG8gdGhlIHJlcXVlc3RlZCBzaGFkZXIgdHlwZS4gV2UgY2FuIGFsd2F5cyBqdXN0IHVzZVxuICogYGNyZWF0ZVByb2dyYW1Gcm9tU291cmNlc2AgdG8gYXZvaWQgY2FsbGluZyB0aGlzIGRpcmVjdGx5LlxuICovXG5leHBvcnQgZnVuY3Rpb24gbG9hZFNoYWRlcihcbiAgICBnbDogV2ViR0xSZW5kZXJpbmdDb250ZXh0LCBzb3VyY2U6IHN0cmluZywgdHlwZTogU2hhZGVyVHlwZSk6IFdlYkdMU2hhZGVyIHtcbiAgY29uc3Qgc2hhZGVyVHlwZSA9XG4gICAgICB0eXBlID09PSBTaGFkZXJUeXBlLlZFUlRFWF9TSEFERVIgPyBnbC5WRVJURVhfU0hBREVSIDogZ2wuRlJBR01FTlRfU0hBREVSO1xuICBjb25zdCBzaGFkZXIgPSBnbC5jcmVhdGVTaGFkZXIoc2hhZGVyVHlwZSkhO1xuICBnbC5zaGFkZXJTb3VyY2Uoc2hhZGVyLCBzb3VyY2UpO1xuICBnbC5jb21waWxlU2hhZGVyKHNoYWRlcik7XG4gIGlmICghZ2wuZ2V0U2hhZGVyUGFyYW1ldGVyKHNoYWRlciwgZ2wuQ09NUElMRV9TVEFUVVMpKSB7XG4gICAgY29uc3QgaW5mbyA9IGdsLmdldFNoYWRlckluZm9Mb2coc2hhZGVyKTtcbiAgICB0aHJvdyBuZXcgRXJyb3IoYENvdWxkIG5vdCBjb21waWxlIFdlYkdMIHNoYWRlci5cXG5cXG4ke2luZm99YCk7XG4gIH1cbiAgcmV0dXJuIHNoYWRlcjtcbn1cblxuLyoqXG4gKiBMaW5rcyBhIHNoYWRlciBwcm9ncmFtIGdpdmVuIHRoZSB2ZXJ0ZXggYW5kIGZyYWdtZW50IHNoYWRlcnMuXG4gKi9cbmV4cG9ydCBmdW5jdGlvbiBjcmVhdGVQcm9ncmFtKFxuICAgIGdsOiBXZWJHTFJlbmRlcmluZ0NvbnRleHQsIHZlcnRleFNoYWRlcjogV2ViR0xTaGFkZXIsXG4gICAgZnJhZ21lbnRTaGFkZXI6IFdlYkdMU2hhZGVyKTogV2ViR0xQcm9ncmFtIHtcbiAgY29uc3QgcHJvZ3JhbSA9IGdsLmNyZWF0ZVByb2dyYW0oKSE7XG4gIGdsLmF0dGFjaFNoYWRlcihwcm9ncmFtLCB2ZXJ0ZXhTaGFkZXIpO1xuICBnbC5hdHRhY2hTaGFkZXIocHJvZ3JhbSwgZnJhZ21lbnRTaGFkZXIpO1xuICBnbC5saW5rUHJvZ3JhbShwcm9ncmFtKTtcbiAgaWYgKCFnbC5nZXRQcm9ncmFtUGFyYW1ldGVyKHByb2dyYW0sIGdsLkxJTktfU1RBVFVTKSkge1xuICAgIGNvbnN0IGluZm8gPSBnbC5nZXRQcm9ncmFtSW5mb0xvZyhwcm9ncmFtKTtcbiAgICB0aHJvdyBuZXcgRXJyb3IoYENvdWxkIG5vdCBjb21waWxlIFdlYkdMIHByb2dyYW0uXFxuXFxuJHtpbmZvfWApO1xuICB9XG4gIHJldHVybiBwcm9ncmFtO1xufVxuXG4vKipcbiAqIEhlbHBlciBmdW5jdGlvbiB0byB0YWtlIHNoYWRlciBwcm9ncmFtcyBhcyBzdHJpbmdzIGFuZCBwcm9kdWNlIGEgbGlua2VkXG4gKiBwcm9ncmFtLlxuICovXG5leHBvcnQgZnVuY3Rpb24gY3JlYXRlUHJvZ3JhbUZyb21Tb3VyY2VzKFxuICAgIGdsOiBXZWJHTFJlbmRlcmluZ0NvbnRleHQsIHZlcnRleFNvdXJjZTogc3RyaW5nLFxuICAgIGZyYWdtZW50U291cmNlOiBzdHJpbmcpOiBXZWJHTFByb2dyYW0ge1xuICBjb25zdCB2ZXJ0ZXhTaGFkZXIgPSBsb2FkU2hhZGVyKGdsLCB2ZXJ0ZXhTb3VyY2UsIFNoYWRlclR5cGUuVkVSVEVYX1NIQURFUik7XG4gIGNvbnN0IGZyYWdtZW50U2hhZGVyID1cbiAgICAgIGxvYWRTaGFkZXIoZ2wsIGZyYWdtZW50U291cmNlLCBTaGFkZXJUeXBlLkZSQUdNRU5UX1NIQURFUik7XG4gIHJldHVybiBjcmVhdGVQcm9ncmFtKGdsLCB2ZXJ0ZXhTaGFkZXIsIGZyYWdtZW50U2hhZGVyKTtcbn1cbiJdfQ==
;return exports;});

//third_party/mediapipe/web/solutions/stream_helpers.closure.js
goog.loadModule(function(exports) {'use strict';/**
 * @fileoverview added by tsickle
 * Generated from: third_party/mediapipe/web/solutions/stream_helpers.ts
 * @suppress {checkTypes,extraRequire,missingOverride,missingRequire,missingReturn,unusedPrivateMembers,uselessCode} checked by tsc
 */
goog.module('google3.third_party.mediapipe.web.solutions.stream_helpers');
var module = module || { id: 'third_party/mediapipe/web/solutions/stream_helpers.closure.js' };
goog.require('google3.third_party.javascript.tslib.tslib');
const tsickle_shader_utils_1 = goog.requireType("google3.third_party.mediapipe.web.solutions.shader_utils");
const tsickle_solutions_wasm_2 = goog.requireType("google3.third_party.mediapipe.web.solutions.solutions_wasm");
const shaderUtils = goog.require('google3.third_party.mediapipe.web.solutions.shader_utils');
/**
 * When the graph is closed or restarted, we use this to give the helpers a
 * chance to cleanup. For instance, OpenGl textures that have been created can
 * be deleted.
 * @record
 */
function StreamHelper() { }
exports.StreamHelper = StreamHelper;
/* istanbul ignore if */
if (false) {
    /**
     * @return {void}
     */
    StreamHelper.prototype.dispose = function () { };
}
/** @type {string} */
const VS_RENDER = `
  attribute vec2 aVertex;
  attribute vec2 aTex;
  varying vec2 vTex;
  void main(void) {
    gl_Position = vec4(aVertex, 0.0, 1.0);
    vTex = aTex;
  }`;
/** @type {string} */
const FS_RENDER = `
  precision highp float;
  varying vec2 vTex;
  uniform sampler2D sampler0;
  void main(){
    gl_FragColor = texture2D(sampler0, vTex);
  }`;
/**
 * @record
 */
function RenderVariableLocations() { }
/* istanbul ignore if */
if (false) {
    /** @type {number} */
    RenderVariableLocations.prototype.aVertex;
    /** @type {number} */
    RenderVariableLocations.prototype.aTex;
    /** @type {!WebGLUniformLocation} */
    RenderVariableLocations.prototype.sampler0;
}
/**
 * Represents a GpuBuffer coming from MediaPipe. We attach helper methods
 * to deal with the internal GL details.
 * @typedef {!HTMLCanvasElement}
 */
exports.GpuBuffer;
/**
 * The types of inputs that can be encoded.
 * @typedef {(!HTMLVideoElement|!HTMLCanvasElement|!HTMLImageElement)}
 */
exports.InputElement;
/**
 * Deals with the trouble of moving image information into MediaPipe back out
 * again. At our lowest level, we use a Canvas element to provide a context in
 * which to do GPU processing, but because browsers do not share contexts
 * between elements, there's no easy way to move texture data around. So we'll
 * do that here. We'll provide ways for the user to supply HTML elements that
 * we'll feed into MediaPipe (see `encode`) and we'll provide ways to render
 * output streams from MediaPipe (see `renderTo`).
 * @implements {StreamHelper}
 */
class ImageStreamHelper {
    /**
     * @param {!tsickle_solutions_wasm_2.MediapipeWasm} wasm
     * @param {!WebGL2RenderingContext} gl
     */
    constructor(wasm, gl) {
        this.wasm = wasm;
        this.gl = gl;
        // This is used for both input_streams and output_streams in a graph. The
        // back end either needs it to wrap an external texture, or provides it when
        // supplying us with a texture (as part of an output stream).
        this.textureId = 0;
    }
    /**
     * Captures a frame from a video element to send as a texture to the graph.
     * TODO(mhays); In the future this will also other elements that supply
     * texture (including HTMLImageElement and HTMLCanvasElement).
     * @param {(!HTMLVideoElement|!HTMLCanvasElement|!HTMLImageElement)} input
     * @return {!Object}
     */
    encode(input) {
        if (this.textureId === 0) {
            this.textureId = this.wasm.createTexture();
        }
        /** @type {number} */
        let width = 0;
        /** @type {number} */
        let height = 0;
        if (input instanceof HTMLVideoElement) {
            width = input.videoWidth;
            height = input.videoHeight;
        }
        else if (input instanceof HTMLImageElement) {
            width = input.naturalWidth;
            height = input.naturalHeight;
        }
        else {
            width = input.width;
            height = input.height;
        }
        /** @type {{glName: number, width: number, height: number}} */
        const result = { glName: this.textureId, width, height };
        /** @type {!WebGL2RenderingContext} */
        const gl = this.gl;
        gl.canvas.width = result.width;
        gl.canvas.height = result.height;
        gl.activeTexture(gl.TEXTURE0);
        this.wasm.bindTexture2d(this.textureId);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, input);
        this.wasm.bindTexture2d(0);
        return result;
    }
    /**
     * @param {!tsickle_solutions_wasm_2.Texture2dData} data
     * @return {!HTMLCanvasElement}
     */
    decode(data) {
        this.render(data);
        return (/** @type {!HTMLCanvasElement} */ (this.gl.canvas));
    }
    /**
     * @private
     * @param {!tsickle_solutions_wasm_2.Texture2dData} data
     * @return {void}
     */
    render(data) {
        /** @type {!WebGL2RenderingContext} */
        const gl = this.gl;
        if (this.renderProgram === undefined) {
            /** @type {!WebGLProgram} */
            const program = this.renderProgram =
                shaderUtils.createProgramFromSources(gl, VS_RENDER, FS_RENDER);
            gl.useProgram(program);
            /** @type {number} */
            const aVertex = gl.getAttribLocation(program, 'aVertex');
            /** @type {number} */
            const aTex = gl.getAttribLocation(program, 'aTex');
            /** @type {!WebGLUniformLocation} */
            const sampler0 = (/** @type {!WebGLUniformLocation} */ (gl.getUniformLocation(program, 'sampler0')));
            this.locations = { aVertex, aTex, sampler0 };
            this.vertexBuffer = (/** @type {!WebGLBuffer} */ (gl.createBuffer()));
            gl.bindBuffer(gl.ARRAY_BUFFER, this.vertexBuffer);
            gl.enableVertexAttribArray(this.locations.aVertex);
            gl.vertexAttribPointer(this.locations.aVertex, 2, gl.FLOAT, false, 0, 0);
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([-1, -1, -1, 1, 1, 1, 1, -1]), gl.STATIC_DRAW);
            gl.bindBuffer(gl.ARRAY_BUFFER, null);
            this.textureBuffer = (/** @type {!WebGLBuffer} */ (gl.createBuffer()));
            gl.bindBuffer(gl.ARRAY_BUFFER, this.textureBuffer);
            gl.enableVertexAttribArray(this.locations.aTex);
            gl.vertexAttribPointer(this.locations.aTex, 2, gl.FLOAT, false, 0, 0);
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([0, 1, 0, 0, 1, 0, 1, 1]), gl.STATIC_DRAW);
            gl.bindBuffer(gl.ARRAY_BUFFER, null);
            gl.uniform1i(sampler0, 0);
        }
        /** @type {!RenderVariableLocations} */
        const locations = (/** @type {!RenderVariableLocations} */ (this.locations));
        /** @type {!WebGLProgram} */
        const program = this.renderProgram;
        gl.useProgram(program);
        gl.canvas.width = data.width;
        gl.canvas.height = data.height;
        gl.viewport(0, 0, data.width, data.height);
        // The texture will already be filled when we are rendering.  We just need
        // to hand it over to the program on sampler0.
        gl.activeTexture(gl.TEXTURE0);
        this.wasm.bindTexture2d(data.glName);
        gl.enableVertexAttribArray(locations.aVertex);
        gl.bindBuffer(gl.ARRAY_BUFFER, (/** @type {!WebGLBuffer} */ (this.vertexBuffer)));
        gl.vertexAttribPointer(locations.aVertex, 2, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(locations.aTex);
        gl.bindBuffer(gl.ARRAY_BUFFER, (/** @type {!WebGLBuffer} */ (this.textureBuffer)));
        gl.vertexAttribPointer(locations.aTex, 2, gl.FLOAT, false, 0, 0);
        // Make sure we're drawing to the canvas and not to a texture.
        gl.bindFramebuffer(gl.DRAW_FRAMEBUFFER, null);
        gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
        gl.disableVertexAttribArray(locations.aVertex);
        gl.disableVertexAttribArray(locations.aTex);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
        this.wasm.bindTexture2d(0);
    }
    /**
     * Disposes of the texture we created. Since we made the texture on the C++
     * layer, we need to delete it there as well.
     * @return {void}
     */
    dispose() {
        if (this.textureId !== 0) {
            this.wasm.deleteTexture(this.textureId);
        }
        if (this.renderProgram) {
            this.gl.deleteProgram(this.renderProgram);
        }
        if (this.textureBuffer) {
            this.gl.deleteBuffer(this.textureBuffer);
        }
        if (this.vertexBuffer) {
            this.gl.deleteBuffer(this.vertexBuffer);
        }
    }
}
exports.ImageStreamHelper = ImageStreamHelper;
/* istanbul ignore if */
if (false) {
    /**
     * @type {number}
     * @private
     */
    ImageStreamHelper.prototype.textureId;
    /**
     * @type {(undefined|!WebGLProgram)}
     * @private
     */
    ImageStreamHelper.prototype.renderProgram;
    /**
     * @type {(undefined|!WebGLBuffer)}
     * @private
     */
    ImageStreamHelper.prototype.vertexBuffer;
    /**
     * @type {(undefined|!WebGLBuffer)}
     * @private
     */
    ImageStreamHelper.prototype.textureBuffer;
    /**
     * @type {(undefined|!RenderVariableLocations)}
     * @private
     */
    ImageStreamHelper.prototype.locations;
    /**
     * @type {!tsickle_solutions_wasm_2.MediapipeWasm}
     * @private
     */
    ImageStreamHelper.prototype.wasm;
    /**
     * @type {!WebGL2RenderingContext}
     * @private
     */
    ImageStreamHelper.prototype.gl;
}
/**
 * Streams sometimes save information aside. For instance, 2d textures will save
 * a texture id.
 * @record
 */
function StreamHelperMap() { }
exports.StreamHelperMap = StreamHelperMap;
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoic3RyZWFtX2hlbHBlcnMuanMiLCJzb3VyY2VSb290IjoiIiwic291cmNlcyI6WyIuLi8uLi8uLi8uLi8uLi8uLi8uLi90aGlyZF9wYXJ0eS9tZWRpYXBpcGUvd2ViL3NvbHV0aW9ucy9zdHJlYW1faGVscGVycy50cyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiOzs7Ozs7Ozs7O0FBQUEsNkZBQThDOzs7Ozs7O0FBUTlDLDJCQUVDOzs7Ozs7O0lBREMsaURBQWdCOzs7TUFHWixTQUFTLEdBQUc7Ozs7Ozs7SUFPZDs7TUFFRSxTQUFTLEdBQUc7Ozs7OztJQU1kOzs7O0FBRUosc0NBSUM7Ozs7SUFIQywwQ0FBZTs7SUFDZix1Q0FBWTs7SUFDWiwyQ0FBK0I7Ozs7Ozs7QUFPakMsa0JBQTBDOzs7OztBQUsxQyxxQkFBK0U7Ozs7Ozs7Ozs7O0FBVy9FLE1BQWEsaUJBQWlCOzs7OztJQVk1QixZQUNxQixJQUFpQyxFQUNqQyxFQUEwQjtRQUQxQixTQUFJLEdBQUosSUFBSSxDQUE2QjtRQUNqQyxPQUFFLEdBQUYsRUFBRSxDQUF3Qjs7OztRQVZ2QyxjQUFTLEdBQUcsQ0FBQyxDQUFDO0lBVTRCLENBQUM7Ozs7Ozs7O0lBT25ELE1BQU0sQ0FBQyxLQUFtQjtRQUN4QixJQUFJLElBQUksQ0FBQyxTQUFTLEtBQUssQ0FBQyxFQUFFO1lBQ3hCLElBQUksQ0FBQyxTQUFTLEdBQUcsSUFBSSxDQUFDLElBQUksQ0FBQyxhQUFhLEVBQUUsQ0FBQztTQUM1Qzs7WUFFRyxLQUFLLEdBQUcsQ0FBQzs7WUFDVCxNQUFNLEdBQUcsQ0FBQztRQUVkLElBQUksS0FBSyxZQUFZLGdCQUFnQixFQUFFO1lBQ3JDLEtBQUssR0FBRyxLQUFLLENBQUMsVUFBVSxDQUFDO1lBQ3pCLE1BQU0sR0FBRyxLQUFLLENBQUMsV0FBVyxDQUFDO1NBQzVCO2FBQU0sSUFBSSxLQUFLLFlBQVksZ0JBQWdCLEVBQUU7WUFDNUMsS0FBSyxHQUFHLEtBQUssQ0FBQyxZQUFZLENBQUM7WUFDM0IsTUFBTSxHQUFHLEtB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QUMsT0FBTyxDQUFDLENBQUM7UUFDL0MsRUFBRSxDQUFDLHdCQUF3QixDQUFDLFNBQVMsQ0FBQyxJQUFJLENBQUMsQ0FBQztRQUM1QyxFQUFFLENBQUMsVUFBVSxDQUFDLEVBQUUsQ0FBQyxZQUFZLEVBQUUsSUFBSSxDQUFDLENBQUM7UUFFckMsSUFBSSxDQUFDLElBQUksQ0FBQyxhQUFhLENBQUMsQ0FBQyxDQUFDLENBQUM7SUFDN0IsQ0FBQzs7Ozs7O0lBTUQsT0FBTztRQUNMLElBQUksSUFBSSxDQUFDLFNBQVMsS0FBSyxDQUFDLEVBQUU7WUFDeEIsSUFBSSxDQUFDLElBQUksQ0FBQyxhQUFhLENBQUMsSUFBSSxDQUFDLFNBQVMsQ0FBQyxDQUFDO1NBQ3pDO1FBQ0QsSUFBSSxJQUFJLENBQUMsYUFBYSxFQUFFO1lBQ3RCLElBQUksQ0FBQyxFQUFFLENBQUMsYUFBYSxDQUFDLElBQUksQ0FBQyxhQUFhLENBQUMsQ0FBQztTQUMzQztRQUNELElBQUksSUFBSSxDQUFDLGFBQWEsRUFBRTtZQUN0QixJQUFJLENBQUMsRUFBRSxDQUFDLFlBQVksQ0FBQyxJQUFJLENBQUMsYUFBYSxDQUFDLENBQUM7U0FDMUM7UUFDRCxJQUFJLElBQUksQ0FBQyxZQUFZLEVBQUU7WUFDckIsSUFBSSxDQUFDLEVBQUUsQ0FBQyxZQUFZLENBQUMsSUFBSSxDQUFDLFlBQVksQ0FBQyxDQUFDO1NBQ3pDO0lBQ0gsQ0FBQztDQUNGO0FBNUlELDhDQTRJQzs7Ozs7OztJQXhJQyxzQ0FBc0I7Ozs7O0lBR3RCLDBDQUFxQzs7Ozs7SUFDckMseUNBQW1DOzs7OztJQUNuQywwQ0FBb0M7Ozs7O0lBQ3BDLHNDQUE0Qzs7Ozs7SUFHeEMsaUNBQWtEOzs7OztJQUNsRCwrQkFBMkM7Ozs7Ozs7QUFvSWpELDhCQUVDIiwic291cmNlc0NvbnRlbnQiOlsiaW1wb3J0ICogYXMgc2hhZGVyVXRpbHMgZnJvbSAnLi9zaGFkZXJfdXRpbHMnO1xuaW1wb3J0ICogYXMgc29sdXRpb25zV2FzbSBmcm9tICcuL3NvbHV0aW9uc193YXNtJztcblxuLyoqXG4gKiBXaGVuIHRoZSBncmFwaCBpcyBjbG9zZWQgb3IgcmVzdGFydGVkLCB3ZSB1c2UgdGhpcyB0byBnaXZlIHRoZSBoZWxwZXJzIGFcbiAqIGNoYW5jZSB0byBjbGVhbnVwLiBGb3IgaW5zdGFuY2UsIE9wZW5HbCB0ZXh0dXJlcyB0aGF0IGhhdmUgYmVlbiBjcmVhdGVkIGNhblxuICogYmUgZGVsZXRlZC5cbiAqL1xuZXhwb3J0IGludGVyZmFjZSBTdHJlYW1IZWxwZXIge1xuICBkaXNwb3NlKCk6IHZvaWQ7XG59XG5cbmNvbnN0IFZTX1JFTkRFUiA9IGBcbiAgYXR0cmlidXRlIHZlYzIgYVZlcnRleDtcbiAgYXR0cmlidXRlIHZlYzIgYVRleDtcbiAgdmFyeWluZyB2ZWMyIHZUZXg7XG4gIHZvaWQgbWFpbih2b2lkKSB7XG4gICAgZ2xfUG9zaXRpb24gPSB2ZWM0KGFWZXJ0ZXgsIDAuMCwgMS4wKTtcbiAgICB2VGV4ID0gYVRleDtcbiAgfWA7XG5cbmNvbnN0IEZTX1JFTkRFUiA9IGBcbiAgcHJlY2lzaW9uIGhpZ2hwIGZsb2F0O1xuICB2YXJ5aW5nIHZlYzIgdlRleDtcbiAgdW5pZm9ybSBzYW1wbGVyMkQgc2FtcGxlcjA7XG4gIHZvaWQgbWFpbigpe1xuICAgIGdsX0ZyYWdDb2xvciA9IHRleHR1cmUyRChzYW1wbGVyMCwgdlRleCk7XG4gIH1gO1xuXG5pbnRlcmZhY2UgUmVuZGVyVmFyaWFibGVMb2NhdGlvbnMge1xuICBhVmVydGV4OiBHTGludDtcbiAgYVRleDogR0xpbnQ7XG4gIHNhbXBsZXIwOiBXZWJHTFVuaWZvcm1Mb2NhdGlvbjtcbn1cblxuLyoqXG4gKiBSZXByZXNlbnRzIGEgR3B1QnVmZmVyIGNvbWluZyBmcm9tIE1lZGlhUGlwZS4gV2UgYXR0YWNoIGhlbHBlciBtZXRob2RzXG4gKiB0byBkZWFsIHdpdGggdGhlIGludGVybmFsIEdMIGRldGFpbHMuXG4gKi9cbmV4cG9ydCB0eXBlIEdwdUJ1ZmZlciA9IEhUTUxDYW52YXNFbGVtZW50O1xuXG4vKipcbiAqIFRoZSB0eXBlcyBvZiBpbnB1dHMgdGhhdCBjYW4gYmUgZW5jb2RlZC5cbiAqL1xuZXhwb3J0IHR5cGUgSW5wdXRFbGVtZW50ID0gSFRNTFZpZGVvRWxlbWVudHxIVE1MQ2FudmFzRWxlbWVudHxIVE1MSW1hZ2VFbGVtZW50O1xuXG4vKipcbiAqIERlYWxzIHdpdGggdGhlIHRyb3VibGUgb2YgbW92aW5nIGltYWdlIGluZm9ybWF0aW9uIGludG8gTWVkaWFQaXBlIGJhY2sgb3V0XG4gKiBhZ2Fpbi4gQXQgb3VyIGxvd2VzdCBsZXZlbCwgd2UgdXNlIGEgQ2FudmFzIGVsZW1lbnQgdG8gcHJvdmlkZSBhIGNvbnRleHQgaW5cbiAqIHdoaWNoIHRvIGRvIEdQVSBwcm9jZXNzaW5nLCBidXQgYmVjYXVzZSBicm93c2VycyBkbyBub3Qgc2hhcmUgY29udGV4dHNcbiAqIGJldHdlZW4gZWxlbWVudHMsIHRoZXJlJ3Mgbm8gZWFzeSB3YXkgdG8gbW92ZSB0ZXh0dXJlIGRhdGEgYXJvdW5kLiBTbyB3ZSdsbFxuICogZG8gdGhhdCBoZXJlLiBXZSdsbCBwcm92aWRlIHdheXMgZm9yIHRoZSB1c2VyIHRvIHN1cHBseSBIVE1MIGVsZW1lbnRzIHRoYXRcbiAqIHdlJ2xsIGZlZWQgaW50byBNZWRpYVBpcGUgKHNlZSBgZW5jb2RlYCkgYW5kIHdlJ2xsIHByb3ZpZGUgd2F5cyB0byByZW5kZXJcbiAqIG91dHB1dCBzdHJlYW1zIGZyb20gTWVkaWFQaXBlIChzZWUgYHJlbmRlclRvYCkuXG4gKi9cbmV4cG9ydCBjbGFzcyBJbWFnZVN0cmVhbUhlbHBlciBpbXBsZW1lbnRzIFN0cmVhbUhlbHBlciB7XG4gIC8vIFRoaXMgaXMgdXNlZCBmb3IgYm90aCBpbnB1dF9zdHJlYW1zIGFuZCBvdXRwdXRfc3RyZWFtcyBpbiBhIGdyYXBoLiBUaGVcbiAgLy8gYmFjayBlbmQgZWl0aGVyIG5lZWRzIGl0IHRvIHdyYXAgYW4gZXh0ZXJuYWwgdGV4dHVyZSwgb3IgcHJvdmlkZXMgaXQgd2hlblxuICAvLyBzdXBwbHlpbmcgdXMgd2l0aCBhIHRleHR1cmUgKGFzIHBhcnQgb2YgYW4gb3V0cHV0IHN0cmVhbSkuXG4gIHByaXZhdGUgdGV4dHVyZUlkID0gMDtcblxuICAvLyBXZWJHTCBwcm9ncmFtcyBhcmUgb25seSB1c2VkIGZvciByZW5kZXJpbmcsIHNvIG9ubHkgZm9yIG91dHB1dCB0ZXh0dXJlcy5cbiAgcHJpdmF0ZSByZW5kZXJQcm9ncmFtPzogV2ViR0xQcm9ncmFtO1xuICBwcml2YXRlIHZlcnRleEJ1ZmZlcj86IFdlYkdMQnVmZmVyO1xuICBwcml2YXRlIHRleHR1cmVCdWZmZXI/OiBXZWJHTEJ1ZmZlcjtcbiAgcHJpdmF0ZSBsb2NhdGlvbnM/OiBSZW5kZXJWYXJpYWJsZUxvY2F0aW9ucztcblxuICBjb25zdHJ1Y3RvcihcbiAgICAgIHByaXZhdGUgcmVhZG9ubHkgd2FzbTogc29sdXRpb25zV2FzbS5NZWRpYXBpcGVXYXNtLFxuICAgICAgcHJpdmF0ZSByZWFkb25seSBnbDogV2ViR0wyUmVuZGVyaW5nQ29udGV4dCkge31cblxuICAvKipcbiAgICogQ2FwdHVyZXMgYSBmcmFtZSBmcm9tIGEgdmlkZW8gZWxlbWVudCB0byBzZW5kIGFzIGEgdGV4dHVyZSB0byB0aGUgZ3JhcGguXG4gICAqIFRPRE8obWhheXMpOyBJbiB0aGUgZnV0dXJlIHRoaXMgd2lsbCBhbHNvIG90aGVyIGVsZW1lbnRzIHRoYXQgc3VwcGx5XG4gICAqIHRleHR1cmUgKGluY2x1ZGluZyBIVE1MSW1hZ2VFbGVtZW50IGFuZCBIVE1MQ2FudmFzRWxlbWVudCkuXG4gICAqL1xuICBlbmNvZGUoaW5wdXQ6IElucHV0RWxlbWVudCk6IG9iamVjdCB7XG4gICAgaWYgKHRoaXMudGV4dHVyZUlkID09PSAwKSB7XG4gICAgICB0aGlzLnRleHR1cmVJZCA9IHRoaXMud2FzbS5jcmVhdGVUZXh0dXJlKCk7XG4gICAgfVxuXG4gICAgbGV0IHdpZHRoID0gMDtcbiAgICBsZXQgaGVpZ2h0ID0gMDtcblxuICAgIGlmIChpbnB1dCBpbnN0YW5jZW9mIEhUTUxWaWRlb0VsZW1lbnQpIHtcbiAgICAgIHdpZHRoID0gaW5wdXQudmlkZW9XaWR0aDtcbiAgICAgIGhlaWdodCA9IGlucHV0LnZpZGVvSGVpZ2h0O1xuICAgIH0gZWxzZSBpZiAoaW5wdXQgaW5zdGFuY2VvZiBIVE1MSW1hZ2VFbGVtZW50KSB7XG4gICAgICB3aWR0aCA9IGlucHV0Lm5hdHVyYWxXaWR0aDtcbiAgICAgIGhlaWdodCA9IGlucHV0Lm5hdHVyYWxIZWlnaHQ7XG4gICAgfSBlbHNlIHtcbiAgICAgIHdpZHRoID0gaW5wdXQud2lkdGg7XG4gICAgICBoZWlnaHQgPSBpbnB1dC5oZWlnaHQ7XG4gICAgfVxuICAgIGNvbnN0IHJlc3VsdCA9IHtnbE5hbWU6IHRoaXMudGV4dHVyZUlkLCB3aWR0aCwgaGVpZ2h0fTtcblxuICAgIGNvbnN0IGdsID0gdGhpcy5nbDtcbiAgICBnbC5jYW52YXMud2lkdGggPSByZXN1bHQud2lkdGg7XG4gICAgZ2wuY2FudmFzLmhlaWdodCA9IHJlc3VsdC5oZWlnaHQ7XG4gICAgZ2wuYWN0aXZlVGV4dHVyZShnbC5URVhUVVJFMCk7XG4gICAgdGhpcy53YXNtLmJpbmRUZXh0dXJlMmQodGhpcy50ZXh0dXJlSWQpO1xuICAgIGdsLnRleEltYWdlMkQoZ2wuVEVYVFVSRV8yRCwgMCwgZ2wuUkdCQSwgZ2wuUkdCQSwgZ2wuVU5TSUdORURfQllURSwgaW5wdXQpO1xuICAgIHRoaXMud2FzbS5iaW5kVGV4dHVyZTJkKDApO1xuICAgIHJldHVybiByZXN1bHQ7XG4gIH1cblxuICBkZWNvZGUoZGF0YTogc29sdXRpb25zV2FzbS5UZXh0dXJlMmREYXRhKTogR3B1QnVmZmVyIHtcbiAgICB0aGlzLnJlbmRlcihkYXRhKTtcbiAgICByZXR1cm4gdGhpcy5nbC5jYW52YXMgYXMgR3B1QnVmZmVyO1xuICB9XG5cbiAgcHJpdmF0ZSByZW5kZXIoZGF0YTogc29sdXRpb25zV2FzbS5UZXh0dXJlMmREYXRhKTogdm9pZCB7XG4gICAgY29uc3QgZ2wgPSB0aGlzLmdsO1xuXG4gICAgaWYgKHRoaXMucmVuZGVyUHJvZ3JhbSA9PT0gdW5kZWZpbmVkKSB7XG4gICAgICBjb25zdCBwcm9ncmFtID0gdGhpcy5yZW5kZXJQcm9ncmFtID1cbiAgICAgICAgICBzaGFkZXJVdGlscy5jcmVhdGVQcm9ncmFtRnJvbVNvdXJjZXMoZ2wsIFZTX1JFTkRFUiwgRlNfUkVOREVSKTtcbiAgICAgIGdsLnVzZVByb2dyYW0ocHJvZ3JhbSk7XG5cbiAgICAgIGNvbnN0IGFWZXJ0ZXggPSBnbC5nZXRBdHRyaWJMb2NhdGlvbihwcm9ncmFtLCAnYVZlcnRleCcpO1xuICAgICAgY29uc3QgYVRleCA9IGdsLmdldEF0dHJpYkxvY2F0aW9uKHByb2dyYW0sICdhVGV4Jyk7XG4gICAgICBjb25zdCBzYW1wbGVyMCA9IGdsLmdldFVuaWZvcm1Mb2NhdGlvbihwcm9ncmFtLCAnc2FtcGxlcjAnKSE7XG4gICAgICB0aGlzLmxvY2F0aW9ucyA9IHthVmVydGV4LCBhVGV4LCBzYW1wbGVyMH07XG5cbiAgICAgIHRoaXMudmVydGV4QnVmZmVyID0gZ2wuY3JlYXRlQnVmZmVyKCkhO1xuICAgICAgZ2wuYmluZEJ1ZmZlcihnbC5BUlJBWV9CVUZGRVIsIHRoaXMudmVydGV4QnVmZmVyKTtcbiAgICAgIGdsLmVuYWJsZVZlcnRleEF0dHJpYkFycmF5KHRoaXMubG9jYXRpb25zLmFWZXJ0ZXgpO1xuICAgICAgZ2wudmVydGV4QXR0cmliUG9pbnRlcih0aGlzLmxvY2F0aW9ucy5hVmVydGV4LCAyLCBnbC5GTE9BVCwgZmFsc2UsIDAsIDApO1xuICAgICAgZ2wuYnVmZmVyRGF0YShcbiAgICAgICAgICBnbC5BUlJBWV9CVUZGRVIsIG5ldyBGbG9hdDMyQXJyYXkoWy0xLCAtMSwgLTEsIDEsIDEsIDEsIDEsIC0xXSksXG4gICAgICAgICAgZ2wuU1RBVElDX0RSQVcpO1xuICAgICAgZ2wuYmluZEJ1ZmZlcihnbC5BUlJBWV9CVUZGRVIsIG51bGwpO1xuXG4gICAgICB0aGlzLnRleHR1cmVCdWZmZXIgPSBnbC5jcmVhdGVCdWZmZXIoKSE7XG4gICAgICBnbC5iaW5kQnVmZmVyKGdsLkFSUkFZX0JVRkZFUiwgdGhpcy50ZXh0dXJlQnVmZmVyKTtcbiAgICAgIGdsLmVuYWJsZVZlcnRleEF0dHJpYkFycmF5KHRoaXMubG9jYXRpb25zLmFUZXgpO1xuICAgICAgZ2wudmVydGV4QXR0cmliUG9pbnRlcih0aGlzLmxvY2F0aW9ucy5hVGV4LCAyLCBnbC5GTE9BVCwgZmFsc2UsIDAsIDApO1xuICAgICAgZ2wuYnVmZmVyRGF0YShcbiAgICAgICAgICBnbC5BUlJBWV9CVUZGRVIsIG5ldyBGbG9hdDMyQXJyYXkoWzAsIDEsIDAsIDAsIDEsIDAsIDEsIDFdKSxcbiAgICAgICAgICBnbC5TVEFUSUNfRFJBVyk7XG4gICAgICBnbC5iaW5kQnVmZmVyKGdsLkFSUkFZX0JVRkZFUiwgbnVsbCk7XG4gICAgICBnbC51bmlmb3JtMWkoc2FtcGxlcjAsIDApO1xuICAgIH1cblxuICAgIGNvbnN0IGxvY2F0aW9ucyA9IHRoaXMubG9jYXRpb25zITtcbiAgICBjb25zdCBwcm9ncmFtID0gdGhpcy5yZW5kZXJQcm9ncmFtO1xuXG4gICAgZ2wudXNlUHJvZ3JhbShwcm9ncmFtKTtcbiAgICBnbC5jYW52YXMud2lkdGggPSBkYXRhLndpZHRoO1xuICAgIGdsLmNhbnZhcy5oZWlnaHQgPSBkYXRhLmhlaWdodDtcbiAgICBnbC52aWV3cG9ydCgwLCAwLCBkYXRhLndpZHRoLCBkYXRhLmhlaWdodCk7XG5cbiAgICAvLyBUaGUgdGV4dHVyZSB3aWxsIGFscmVhZHkgYmUgZmlsbGVkIHdoZW4gd2UgYXJlIHJlbmRlcmluZy4gIFdlIGp1c3QgbmVlZFxuICAgIC8vIHRvIGhhbmQgaXQgb3ZlciB0byB0aGUgcHJvZ3JhbSBvbiBzYW1wbGVyMC5cbiAgICBnbC5hY3RpdmVUZXh0dXJlKGdsLlRFWFRVUkUwKTtcbiAgICB0aGlzLndhc20uYmluZFRleHR1cmUyZChkYXRhLmdsTmFtZSk7XG5cbiAgICBnbC5lbmFibGVWZXJ0ZXhBdHRyaWJBcnJheShsb2NhdGlvbnMuYVZlcnRleCk7XG4gICAgZ2wuYmluZEJ1ZmZlcihnbC5BUlJBWV9CVUZGRVIsIHRoaXMudmVydGV4QnVmZmVyISk7XG4gICAgZ2wudmVydGV4QXR0cmliUG9pbnRlcihsb2NhdGlvbnMuYVZlcnRleCwgMiwgZ2wuRkxPQVQsIGZhbHNlLCAwLCAwKTtcblxuICAgIGdsLmVuYWJsZVZlcnRleEF0dHJpYkFycmF5KGxvY2F0aW9ucy5hVGV4KTtcbiAgICBnbC5iaW5kQnVmZmVyKGdsLkFSUkFZX0JVRkZFUiwgdGhpcy50ZXh0dXJlQnVmZmVyISk7XG4gICAgZ2wudmVydGV4QXR0cmliUG9pbnRlcihsb2NhdGlvbnMuYVRleCwgMiwgZ2wuRkxPQVQsIGZhbHNlLCAwLCAwKTtcblxuICAgIC8vIE1ha2Ugc3VyZSB3ZSdyZSBkcmF3aW5nIHRvIHRoZSBjYW52YXMgYW5kIG5vdCB0byBhIHRleHR1cmUuXG4gICAgZ2wuYmluZEZyYW1lYnVmZmVyKGdsLkRSQVdfRlJBTUVCVUZGRVIsIG51bGwpO1xuXG4gICAgZ2wuZHJhd0FycmF5cyhnbC5UUklBTkdMRV9GQU4sIDAsIDQpO1xuXG4gICAgZ2wuZGlzYWJsZVZlcnRleEF0dHJpYkFycmF5KGxvY2F0aW9ucy5hVmVydGV4KTtcbiAgICBnbC5kaXNhYmxlVmVydGV4QXR0cmliQXJyYXkobG9jYXRpb25zLmFUZXgpO1xuICAgIGdsLmJpbmRCdWZmZXIoZ2wuQVJSQVlfQlVGRkVSLCBudWxsKTtcblxuICAgIHRoaXMud2FzbS5iaW5kVGV4dHVyZTJkKDApO1xuICB9XG5cbiAgLyoqXG4gICAqIERpc3Bvc2VzIG9mIHRoZSB0ZXh0dXJlIHdlIGNyZWF0ZWQuIFNpbmNlIHdlIG1hZGUgdGhlIHRleHR1cmUgb24gdGhlIEMrK1xuICAgKiBsYXllciwgd2UgbmVlZCB0byBkZWxldGUgaXQgdGhlcmUgYXMgd2VsbC5cbiAgICovXG4gIGRpc3Bvc2UoKTogdm9pZCB7XG4gICAgaWYgKHRoaXMudGV4dHVyZUlkICE9PSAwKSB7XG4gICAgICB0aGlzLndhc20uZGVsZXRlVGV4dHVyZSh0aGlzLnRleHR1cmVJZCk7XG4gICAgfVxuICAgIGlmICh0aGlzLnJlbmRlclByb2dyYW0pIHtcbiAgICAgIHRoaXMuZ2wuZGVsZXRlUHJvZ3JhbSh0aGlzLnJlbmRlclByb2dyYW0pO1xuICAgIH1cbiAgICBpZiAodGhpcy50ZXh0dXJlQnVmZmVyKSB7XG4gICAgICB0aGlzLmdsLmRlbGV0ZUJ1ZmZlcih0aGlzLnRleHR1cmVCdWZmZXIpO1xuICAgIH1cbiAgICBpZiAodGhpcy52ZXJ0ZXhCdWZmZXIpIHtcbiAgICAgIHRoaXMuZ2wuZGVsZXRlQnVmZmVyKHRoaXMudmVydGV4QnVmZmVyKTtcbiAgICB9XG4gIH1cbn1cblxuLyoqXG4gKiBTdHJlYW1zIHNvbWV0aW1lcyBzYXZlIGluZm9ybWF0aW9uIGFzaWRlLiBGb3IgaW5zdGFuY2UsIDJkIHRleHR1cmVzIHdpbGwgc2F2ZVxuICogYSB0ZXh0dXJlIGlkLlxuICovXG5leHBvcnQgaW50ZXJmYWNlIFN0cmVhbUhlbHBlck1hcCB7XG4gIFtrZXk6IHN0cmluZ106IFN0cmVhbUhlbHBlcjtcbn1cbiJdfQ==
;return exports;});

//third_party/mediapipe/web/solutions/solutions_api.closure.js
goog.loadModule(function(exports) {'use strict';/**
 * @fileoverview added by tsickle
 * Generated from: third_party/mediapipe/web/solutions/solutions_api.ts
 * @suppress {checkTypes,extraRequire,missingOverride,missingRequire,missingReturn,unusedPrivateMembers,uselessCode} checked by tsc
 */
goog.module('google3.third_party.mediapipe.web.solutions.solutions_api');
var module = module || { id: 'third_party/mediapipe/web/solutions/solutions_api.closure.js' };
const tslib_1 = goog.require('google3.third_party.javascript.tslib.tslib');
const tsickle_solutions_wasm_1 = goog.requireType("google3.third_party.mediapipe.web.solutions.solutions_wasm");
const tsickle_stream_helpers_2 = goog.requireType("google3.third_party.mediapipe.web.solutions.stream_helpers");
const streamHelpers = goog.require('google3.third_party.mediapipe.web.solutions.stream_helpers');
/**
 * Represents pairs of (start,end) indexes so that we can connect landmarks
 * with lines to provide a skeleton when we draw the points. These are defined
 * within each specific solution, but a common definition is held here for
 * convenience.
 * @typedef {!Array<!Array<?>>}
 */
exports.LandmarkConnectionArray;
/**
 * Represents a normalized rectangle that can be passed to a StreamListener.
 * @typedef {!tsickle_solutions_wasm_1.NormalizedRect}
 */
exports.NormalizedRect;
/**
 * Represents a list of normalized rectangles.
 * @typedef {!Array<!tsickle_solutions_wasm_1.NormalizedRect>}
 */
exports.NormalizedRectList;
/**
 * Represents a detection.
 * @record
 */
function Detection() { }
exports.Detection = Detection;
/* istanbul ignore if */
if (false) {
    /**
     * The bounding box around the detection.
     * @type {!tsickle_solutions_wasm_1.NormalizedRect}
     */
    Detection.prototype.boundingBox;
    /**
     * The associated landmarks.
     *
     * May be empty. This is typically filled for e.g. face, hands or pose
     * detection use cases.
     * @type {!Array<!NormalizedLandmark>}
     */
    Detection.prototype.landmarks;
    /**
     * The associated classifications.
     *
     * May be empty. This is typically filled for e.g. object detection use cases.
     * @type {!Array<!Classification>}
     */
    Detection.prototype.classifications;
}
/**
 * Represents a list of detections.
 * @typedef {!Array<!Detection>}
 */
exports.DetectionList;
/**
 * Represents a detected landmark.
 * @record
 */
function NormalizedLandmark() { }
exports.NormalizedLandmark = NormalizedLandmark;
/* istanbul ignore if */
if (false) {
    /** @type {number} */
    NormalizedLandmark.prototype.x;
    /** @type {number} */
    NormalizedLandmark.prototype.y;
    /** @type {number} */
    NormalizedLandmark.prototype.z;
    /** @type {(undefined|number)} */
    NormalizedLandmark.prototype.visibility;
}
/**
 * Represents a list of landmarks.
 * @typedef {!Array<!NormalizedLandmark>}
 */
exports.NormalizedLandmarkList;
/**
 * Represents a list of lists of landmarks.
 * @typedef {!Array<!Array<!NormalizedLandmark>>}
 */
exports.NormalizedLandmarkListList;
/**
 * A class label with score.
 * @record
 */
function Classification() { }
exports.Classification = Classification;
/* istanbul ignore if */
if (false) {
    /**
     * The index of the class in the corresponding label map.
     * @type {number}
     */
    Classification.prototype.index;
    /**
     * The probability score for this class.
     * @type {number}
     */
    Classification.prototype.score;
    /**
     * Label or name of the class.
     * @type {(undefined|string)}
     */
    Classification.prototype.label;
    /**
     * Display name of the class, for visualization purposes.
     * @type {(undefined|string)}
     */
    Classification.prototype.displayName;
}
/**
 * A list of classifications.
 * @typedef {!Array<!Classification>}
 */
exports.ClassificationList;
/**
 * Multiple lists of classifications.
 * @typedef {!Array<!Array<!Classification>>}
 */
exports.ClassificationListList;
/**
 * Represents a single normalized landmark, with depth.
 * @record
 */
function Point2D() { }
exports.Point2D = Point2D;
/* istanbul ignore if */
if (false) {
    /** @type {number} */
    Point2D.prototype.x;
    /** @type {number} */
    Point2D.prototype.y;
    /** @type {number} */
    Point2D.prototype.depth;
}
/**
 * Represents a point in 3D space.
 * @record
 */
function Point3D() { }
exports.Point3D = Point3D;
/* istanbul ignore if */
if (false) {
    /** @type {number} */
    Point3D.prototype.x;
    /** @type {number} */
    Point3D.prototype.y;
    /** @type {number} */
    Point3D.prototype.z;
}
/**
 * Represents a particular keypoint from an object detection. The id denotes
 * which keypoint this is (consistent over time), and then two representations
 * are included for this point, a 2D normalized version, and a 3D version.
 * @record
 */
function Keypoint() { }
exports.Keypoint = Keypoint;
/* istanbul ignore if */
if (false) {
    /** @type {number} */
    Keypoint.prototype.id;
    /** @type {!Point3D} */
    Keypoint.prototype.point3d;
    /** @type {!Point2D} */
    Keypoint.prototype.point2d;
}
/**
 * List of keypoints.
 * @typedef {!Array<!Keypoint>}
 */
exports.KeypointList;
/**
 * The relevant information about a particular object detection.
 * @record
 */
function ObjectDetection() { }
exports.ObjectDetection = ObjectDetection;
/* istanbul ignore if */
if (false) {
    /**
     * The unique object instance identifier.
     * @type {number}
     */
    ObjectDetection.prototype.id;
    /**
     * The list of keypoints for this detection (8 for a 3D bounding box).
     * @type {!Array<!Keypoint>}
     */
    ObjectDetection.prototype.keypoints;
    /**
     * The visibility of this object detection in a frame.
     * @type {number}
     */
    ObjectDetection.prototype.visibility;
}
/**
 * List of object detections.
 * @typedef {!Array<!ObjectDetection>}
 */
exports.ObjectDetectionList;
/**
 * Represents an image produced by the graph.
 * @typedef {!HTMLCanvasElement}
 */
exports.GpuBuffer;
/**
 * Represents a single result of a multi result listener. The shape is
 * transformed into a shape that the end user can consume.
 * @record
 */
function ResultMap() { }
exports.ResultMap = ResultMap;
/**
 * Represents a single result of a multi result listener. The shape is the
 * wasm class from which we can pull out different kinds of data.
 * @record
 */
function WasmResultMap() { }
exports.WasmResultMap = WasmResultMap;
/**
 * This represents the callbacks from the graph's stream listeners. If the
 * listener needs to do asynchronous work, return a Promise to allow the
 * solution to wait.
 * @typedef {function(!ResultMap): (void|!Promise<void>)}
 */
exports.ResultMapListener;
/**
 * Option descriptions that describe where to change an option in a graph.
 * @typedef {!tsickle_solutions_wasm_1.GraphOptionXRef}
 */
exports.GraphOptionXRef;
/**
 * If your files are organized in some weird structure, you can customize how
 * the path is constructed by sending this to the config options for Solution.
 * @typedef {function(string, string): string}
 */
exports.FileLocatorFn;
/**
 * Users of the solution set options using a key-value map. However, specific
 * solutions will constrain the options with an exported interface.
 *
 * e.g.
 *
 * generalSolution.setOptions({"myKey": myValue}) becomes
 * specificSolution.setOptions({ myKey: myValue }}
 *
 * declare interface SpecificSolutionOptionList {
 *   myKey: number
 * };
 *
 * This works because `Object`s in JavaScript are just hashmaps, and we fit
 * meaning on top of those hashmaps with our type declarations.
 * @record
 */
function OptionList() { }
exports.OptionList = OptionList;
/**
 * Used in the constructor of Graph.
 * @record
 */
function Graph() { }
exports.Graph = Graph;
/* istanbul ignore if */
if (false) {
    /** @type {string} */
    Graph.prototype.url;
}
/**
 * Describes how to turn user input (See InputMap) into data the graph can
 * consume.
 * @record
 */
function InputConfig() { }
exports.InputConfig = InputConfig;
/* istanbul ignore if */
if (false) {
    /**
     * Provides direction on how to interpret the `value` of the InputMap.
     * @type {string}
     */
    InputConfig.prototype.type;
    /**
     * The name of the input stream of the graph that will receive the input.
     * @type {string}
     */
    InputConfig.prototype.stream;
}
/**
 * The full description of all the inputs that will be exposed to the user. The
 * key is the name of the input a user will use.
 * @record
 */
function InputConfigMap() { }
exports.InputConfigMap = InputConfigMap;
/**
 * Describes the inputs that the user will send to the graph. `unknown` allows
 * users to do all sorts of things, from using HTMLVideoElements to numbers to
 * whatever else we need. See `prepareInput` to see how this unknown is turned
 * into structural data that can be sent to the graph.
 *
 * The specific solution should supply a stronger interface that will be used
 * instead of this generic keymap,  In this way, the solution designer can
 * name very specific inputs and exactly what is expected in that InputMap.'
 * @record
 */
function InputMap() { }
exports.InputMap = InputMap;
/**
 * We support several ways to get image inputs.
 * @typedef {(!HTMLVideoElement|!HTMLCanvasElement|!HTMLImageElement)}
 */
exports.InputImage;
/** @enum {number} */
const OptionType = {
    NUMBER: 0,
    BOOL: 1,
};
exports.OptionType = OptionType;
OptionType[OptionType.NUMBER] = 'NUMBER';
OptionType[OptionType.BOOL] = 'BOOL';
/**
 * Used to register keys with the solution. Right now it is limited to
 * graphOptionXref, which is specifically for options that will be fed to the
 * MediaPipe graph, but we'll also need options that are handled by the specific
 * solution to do work (e.g., selfie-flipping).
 * @record
 */
function OptionConfig() { }
exports.OptionConfig = OptionConfig;
/* istanbul ignore if */
if (false) {
    /** @type {!OptionType} */
    OptionConfig.prototype.type;
    /** @type {(undefined|!tsickle_solutions_wasm_1.GraphOptionXRef)} */
    OptionConfig.prototype.graphOptionXref;
}
/**
 * Optionally specified in the output to transform the output.
 * @typedef {function(?): ?}
 */
exports.Transformer;
/**
 * Output configuration without a transform function.
 * @record
 */
function OutputConfigBase() { }
exports.OutputConfigBase = OutputConfigBase;
/* istanbul ignore if */
if (false) {
    /** @type {string} */
    OutputConfigBase.prototype.stream;
    /** @type {string} */
    OutputConfigBase.prototype.type;
    /** @type {(undefined|function(?): ?)} */
    OutputConfigBase.prototype.transform;
}
/**
 * Output config with type safety for a list of landmarks.
 * @record
 * @template T
 * @extends {OutputConfigBase}
 */
function OutputConfigLandmarkList() { }
exports.OutputConfigLandmarkList = OutputConfigLandmarkList;
/* istanbul ignore if */
if (false) {
    /** @type {string} */
    OutputConfigLandmarkList.prototype.type;
    /** @type {(undefined|function(!Array<!NormalizedLandmark>): T)} */
    OutputConfigLandmarkList.prototype.transform;
}
/**
 * Output config with type safety for a list of landmark lists.
 * @record
 * @template T
 * @extends {OutputConfigBase}
 */
function OutputConfigLandmarkListList() { }
exports.OutputConfigLandmarkListList = OutputConfigLandmarkListList;
/* istanbul ignore if */
if (false) {
    /** @type {string} */
    OutputConfigLandmarkListList.prototype.type;
    /** @type {(undefined|function(!Array<!Array<!NormalizedLandmark>>): T)} */
    OutputConfigLandmarkListList.prototype.transform;
}
/**
 * Output config with type safety for a list of classification lists.
 * @record
 * @template T
 * @extends {OutputConfigBase}
 */
function OutputConfigClassificationListList() { }
exports.OutputConfigClassificationListList = OutputConfigClassificationListList;
/* istanbul ignore if */
if (false) {
    /** @type {string} */
    OutputConfigClassificationListList.prototype.type;
    /** @type {(undefined|function(!Array<!Array<!Classification>>): T)} */
    OutputConfigClassificationListList.prototype.transform;
}
/**
 * Output config with type safety for an object detection.
 * @record
 * @template T
 * @extends {OutputConfigBase}
 */
function OutputConfigObjectDetectionList() { }
exports.OutputConfigObjectDetectionList = OutputConfigObjectDetectionList;
/* istanbul ignore if */
if (false) {
    /** @type {string} */
    OutputConfigObjectDetectionList.prototype.type;
    /** @type {(undefined|function(!Array<!ObjectDetection>): T)} */
    OutputConfigObjectDetectionList.prototype.transform;
}
/**
 * When specifying outputs, cast to OutputConfig<T> to get type safety between
 * the `type` and the `transform`.
 * @typedef {(!OutputConfigLandmarkList<?>|!OutputConfigLandmarkListList<?>|!OutputConfigClassificationListList<?>|!OutputConfigObjectDetectionList<?>)}
 */
exports.OutputConfig;
/**
 * Translation map to expose underlying streams to the user. A solution
 * designer should declare a type to constrain the shape of the outputs.
 * @record
 */
function OutputConfigMap() { }
exports.OutputConfigMap = OutputConfigMap;
/**
 * The collection of option configuration entries, arranged by option name.
 * @record
 */
function OptionConfigMap() { }
exports.OptionConfigMap = OptionConfigMap;
/**
 * Binds the streams to a set of listeners that can be surfaced to the
 * developer.
 * @record
 */
function ListenerPort() { }
exports.ListenerPort = ListenerPort;
/* istanbul ignore if */
if (false) {
    /**
     * If there is only one registration, the name can be omitted. See onResults
     * for more discussion.
     * @type {(undefined|string)}
     */
    ListenerPort.prototype.name;
    /**
     * A list of streams in the graph that will be combined into a HashMap
     * which will be assembled and delivered to listeners that the end user
     * can listen to.
     * @type {!Array<string>}
     */
    ListenerPort.prototype.wants;
    /**
     * Allows the results to be transformed into literally any kind of shape. It
     * is up to the solution creator to expose this type information through
     * an onResults specialization.
     * @type {(undefined|!OutputConfigMap)}
     */
    ListenerPort.prototype.outs;
    /**
     * Allows transformation of `outs` before handing the results back to the
     * listener. Allows final creative control of the shape of the output.
     * @type {(undefined|function(!OutputConfigMap): !OutputConfigMap)}
     */
    ListenerPort.prototype.transform;
}
/**
 * Describes a file that should be loaded by the solution. We can include flags
 * for checks that should be done before loading a file. Currently, we allow
 * a user to provide special files that are only provided if SIMD is
 * specifically supported or not supported by the browser (do not include the
 * simd flag if you do not care).
 * @record
 */
function FileEntry() { }
exports.FileEntry = FileEntry;
/* istanbul ignore if */
if (false) {
    /** @type {(undefined|boolean)} */
    FileEntry.prototype.simd;
    /** @type {string} */
    FileEntry.prototype.url;
}
/**
 * Options to configure the solution.
 * @record
 */
function SolutionConfig() { }
exports.SolutionConfig = SolutionConfig;
/* istanbul ignore if */
if (false) {
    /**
     * The pack loader and the wasm loader need to be here. A file loader will be
     * provided to them, and They will get loaded asynchronously. We won't
     * continue initialization until they've completely loaded and run.
     * @type {!ReadonlyArray<!FileEntry>}
     */
    SolutionConfig.prototype.files;
    /**
     * The binary graph. Can support multiple ways of getting that graph.
     * @type {!Graph}
     */
    SolutionConfig.prototype.graph;
    /**
     * Contains a mapping that allows us to turn the inputs a user sends as part
     * of `send` into an input that the graph can consume.
     * @type {!InputConfigMap}
     */
    SolutionConfig.prototype.inputs;
    /**
     * See FileLocatorFn. Any file loading done on the user's behalf will use
     * locateFile if its profived. This includes WASM blobs and graph urls.
     * @type {(undefined|function(string, string): string)}
     */
    SolutionConfig.prototype.locateFile;
    /**
     * Specifies how to interpret options fed to setOptions.
     * @type {(undefined|!OptionConfigMap)}
     */
    SolutionConfig.prototype.options;
    /**
     * Attaches listeners to the graph. This is only for registering the ports.
     * To actually register a callback, the developer will call onResults with
     * the name of the port they want to tap into.
     * @type {(undefined|!Array<!ListenerPort>)}
     */
    SolutionConfig.prototype.listeners;
}
/**
 * Represents a graph that can be fed to mediapipe Solution.
 * @record
 */
function GraphData() { }
exports.GraphData = GraphData;
/* istanbul ignore if */
if (false) {
    /**
     * @return {!Promise<!ArrayBuffer>}
     */
    GraphData.prototype.toArrayBuffer = function () { };
}
// These export names come from wasm_cc_binary BUILD rules. They belong to two
// different scripts that are loaded in parallel (see Promise.all, below).
// Because they mutate their respective variables, there is a race condition
// where they will stomp on each other if they choose the same name. Users
// won't normally see this race condition because they put script tags in the
// HTML, and HTML guarantees that the scripts will be run in order.
/** @type {string} */
const API_EXPORT_NAME = 'createMediapipeSolutionsWasm';
/** @type {string} */
const PACK_EXPORT_NAME = 'createMediapipeSolutionsPackedAssets';
// Used internally to represent a port for which the user did not define a name.
/** @type {string} */
const DEFAULT_PORT_NAME = '$';
/**
 * Simplest WASM program to perform a SIMD operation. We'll try to instantiate
 * it with WebAssembly.instantiate() and if it fails, we'll know there is no
 * SIMD support.
 *
 * To generate the web assembly, I included the following script into a web page
 * https://cdn.jsdelivr.net/gh/AssemblyScript/wabt.js/index.js
 *
 * And then followed it with a bit of script:
 *
 * ```
 * async function run() {
 * var wasm = `(module
 *  (func
 *      i32.const 0
 *      i8x16.splat
 *      drop
 *    )
 *  )`;
 *  const wabt = await WabtModule();
 *  const wat = wabt.parseWat("test", wasm, {simd: true});
 *  const bin = wat.toBinary({}).buffer;
 *
 *  // The following line prints the array that we cut and paste into our source
 *  // code.
 *  console.log(bin.join(','));
 *
 *  const arr = new Uint8Array(bin);
 *  const wa = await WebAssembly.instantiate(arr);
 * }
 * run();
 * ```
 * @type {!Uint8Array}
 */
const WASM_SIMD_CHECK = new Uint8Array([
    0, 97, 115, 109, 1, 0, 0, 0, 1, 4, 1, 96, 0, 0, 3,
    2, 1, 0, 10, 9, 1, 7, 0, 65, 0, 253, 15, 26, 11
]);
/**
 * Returns the default path to a resource if the user did not overload the
 * locateFile parameter in SolutionConfig.
 * @param {string} file
 * @param {string} prefix
 * @return {string}
 */
function defaultLocateFile(file, prefix) {
    return prefix + file;
}
/**
 * Sets an arbitrary value on `window`. This helper function gets around a lot
 * of the cumbersome boilerplate required to set and retrieve arbitrary data
 * from the `window` object.
 * @template T
 * @param {string} key
 * @param {T} value
 * @return {void}
 */
function setWindow(key, value) {
    ((/** @type {!Object<string,T>} */ ((/** @type {*} */ (window)))))[key] = value;
}
/**
 * Gets an arbitrary value from `window`. See `setWindow`.
 * @param {string} key
 * @return {*}
 */
function getWindow(key) {
    return ((/** @type {!Object<string,*>} */ ((/** @type {*} */ (window)))))[key];
}
/**
 * Dynamically loads a ascript into the current page and returns a `Promise`
 * that resolves when its loading is complete.
 * @param {string} url
 * @return {!Promise<void>}
 */
function loadScript(url) {
    /** @type {!HTMLScriptElement} */
    const script = document.createElement('script');
    script.setAttribute('src', url);
    script.setAttribute('crossorigin', 'anonymous');
    document.body.appendChild(script);
    return new Promise((/**
     * @param {function((void|!PromiseLike<void>)): void} resolve
     * @return {void}
     */
    (resolve) => {
        script.addEventListener('load', (/**
         * @return {void}
         */
        () => {
            resolve();
        }), false);
    }));
}
/**
 * @return {string}
 */
function getPackagePath() {
    if (typeof window === 'object') {
        return window.location.pathname.toString().substring(0, window.location.pathname.toString().lastIndexOf('/')) +
            '/';
    }
    else if (typeof location !== 'undefined') {
        // worker
        return location.pathname.toString().substring(0, location.pathname.toString().lastIndexOf('/')) +
            '/';
    }
    else {
        throw new Error('solutions can only be loaded on a web page or in a web worker');
    }
}
/**
 * @param {!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.NormalizedLandmark>} landmarks
 * @return {!Array<!NormalizedLandmark>}
 */
function decodeLandmarks(landmarks) {
    /** @type {!Array<?>} */
    const output = [];
    /** @type {number} */
    const resultCount = landmarks.size();
    for (let index = 0; index < resultCount; ++index) {
        /** @type {!tsickle_solutions_wasm_1.NormalizedLandmark} */
        const landmark = landmarks.get(index);
        output.push({
            x: landmark.x,
            y: landmark.y,
            z: landmark.z,
            visibility: landmark.hasVisibility ? landmark.visibility : undefined
        });
    }
    return output;
}
/**
 * @param {!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.NormalizedRect>} rects
 * @return {!Array<!tsickle_solutions_wasm_1.NormalizedRect>}
 */
function decodeRectList(rects) {
    /** @type {!Array<?>} */
    const output = [];
    /** @type {number} */
    const resultCount = rects.size();
    for (let index = 0; index < resultCount; ++index) {
        /** @type {!tsickle_solutions_wasm_1.NormalizedRect} */
        const rect = rects.get(index);
        output.push(rect);
    }
    return output;
}
/**
 * @param {!tsickle_solutions_wasm_1.ResultWasm} results
 * @return {!Array<!Detection>}
 */
function decodeDetectionList(results) {
    /** @type {!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.NormalizedRect>} */
    const rectList = (/** @type {!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.NormalizedRect>} */ (results.getRectList()));
    /** @type {!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.NormalizedLandmark>>} */
    const landmarksList = (/** @type {!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.NormalizedLandmark>>} */ (results.getLandmarksList()));
    /** @type {!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.Classification>>} */
    const classificationsList = (/** @type {!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.Classification>>} */ (results.getClassificationsList()));
    /** @type {!Array<?>} */
    const output = [];
    if (rectList) {
        for (let index = 0; index < rectList.size(); ++index) {
            /** @type {{boundingBox: !tsickle_solutions_wasm_1.NormalizedRect, landmarks: !Array<!NormalizedLandmark>, classifications: !Array<!Classification>}} */
            const result = {
                boundingBox: rectList.get(index),
                landmarks: decodeLandmarks(landmarksList.get(index)),
                classifications: decodeClassifications(classificationsList.get(index)),
            };
            output.push(result);
        }
    }
    return output;
}
/**
 * @param {!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.NormalizedLandmark>>} landmarks
 * @return {!Array<!Array<!NormalizedLandmark>>}
 */
function decodeLandmarksList(landmarks) {
    /** @type {!Array<?>} */
    const output = [];
    /** @type {number} */
    const resultCount = landmarks.size();
    for (let index = 0; index < resultCount; ++index) {
        /** @type {!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.NormalizedLandmark>} */
        const landmark = landmarks.get(index);
        output.push(decodeLandmarks(landmark));
    }
    return output;
}
/**
 * @param {!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.Classification>} classifications
 * @return {!Array<!Classification>}
 */
function decodeClassifications(classifications) {
    /** @type {!Array<?>} */
    const output = [];
    /** @type {number} */
    const resultCount = classifications.size();
    for (let index = 0; index < resultCount; ++index) {
        /** @type {!tsickle_solutions_wasm_1.Classification} */
        const classification = classifications.get(index);
        output.push({
            index: classification.index,
            score: classification.score,
            label: classification.hasLabel ? classification.label : undefined,
            displayName: classification.hasDisplayName ? classification.displayName :
                undefined,
        });
    }
    return output;
}
/**
 * @param {!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.Classification>>} classifications
 * @return {!Array<!Array<!Classification>>}
 */
function decodeClassificationsList(classifications) {
    /** @type {!Array<?>} */
    const output = [];
    /** @type {number} */
    const resultCount = classifications.size();
    for (let index = 0; index < resultCount; ++index) {
        /** @type {!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.Classification>} */
        const classification = classifications.get(index);
        output.push(decodeClassifications(classification));
    }
    return output;
}
/**
 * @param {!tsickle_solutions_wasm_1.Keypoint} keypoint
 * @return {!Keypoint}
 */
function decodeKeypoint(keypoint) {
    return {
        id: keypoint.id,
        point3d: {
            x: keypoint.point3d.x,
            y: keypoint.point3d.y,
            z: keypoint.point3d.z,
        },
        point2d: {
            x: keypoint.point2d.x,
            y: keypoint.point2d.y,
            depth: keypoint.point2d.depth,
        }
    };
}
/**
 * @param {!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.Keypoint>} keypoints
 * @return {!Array<!Keypoint>}
 */
function decodeKeypointList(keypoints) {
    /** @type {!Array<?>} */
    const output = [];
    /** @type {number} */
    const resultCount = keypoints.size();
    for (let index = 0; index < resultCount; ++index) {
        /** @type {!tsickle_solutions_wasm_1.Keypoint} */
        const keypoint = keypoints.get(index);
        output.push(decodeKeypoint(keypoint));
    }
    return output;
}
/**
 * @param {!tsickle_solutions_wasm_1.ObjectDetection} detection
 * @return {!ObjectDetection}
 */
function decodeObjectDetection(detection) {
    return {
        id: detection.id,
        keypoints: decodeKeypointList(detection.keypoints),
        visibility: detection.visibility,
    };
}
/**
 * @param {!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.ObjectDetection>} detections
 * @return {!Array<!ObjectDetection>}
 */
function decodeObjectDetectionList(detections) {
    /** @type {!Array<?>} */
    const output = [];
    /** @type {number} */
    const resultCount = detections.size();
    for (let index = 0; index < resultCount; ++index) {
        /** @type {!tsickle_solutions_wasm_1.ObjectDetection} */
        const detection = detections.get(index);
        output.push(decodeObjectDetection(detection));
    }
    return output;
}
class GraphDataImpl {
    /**
     * @param {!Graph} graph
     * @param {function(string, string): string} locateFile
     * @param {string} packagePath
     */
    constructor(graph, locateFile, packagePath) {
        this.graph = graph;
        this.locateFile = locateFile;
        this.packagePath = packagePath;
    }
    /**
     * @return {!Promise<!ArrayBuffer>}
     */
    toArrayBuffer() {
        return tslib_1.__awaiter(this, void 0, void 0, function* () {
            if (this.graph.url) {
                /** @type {!Response} */
                const fetched = yield fetch(this.locateFile(this.graph.url, this.packagePath));
                if (fetched.body) {
                    return fetched.arrayBuffer();
                }
            }
            return new ArrayBuffer(0);
        });
    }
}
/* istanbul ignore if */
if (false) {
    /**
     * @type {!Graph}
     * @private
     */
    GraphDataImpl.prototype.graph;
    /**
     * @type {function(string, string): string}
     * @private
     */
    GraphDataImpl.prototype.locateFile;
    /**
     * @type {string}
     * @private
     */
    GraphDataImpl.prototype.packagePath;
}
/**
 * Inputs to the graph. Currently only one input, a video frame, is
 * permitted, but this should encompass any input data to a solution.
 * @record
 */
function FrameInputs() { }
exports.FrameInputs = FrameInputs;
/* istanbul ignore if */
if (false) {
    /** @type {!HTMLVideoElement} */
    FrameInputs.prototype.video;
}
/**
 * The solution keeps all of the current listeners. The user can pass us a
 * listener map and then change the elements at any time. They will be picked
 * up on the next pass through `processFrame`.
 * @record
 */
function ListenerMap() { }
exports.ListenerMap = ListenerMap;
/**
 * @return {!Promise<boolean>}
 * @this {*}
 */
function isSupportedSimd() {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        try {
            yield WebAssembly.instantiate(WASM_SIMD_CHECK);
        }
        catch (_a) {
            return false;
        }
        return true;
    });
}
/**
 * MediaPipe Solution upon which all specific solutions will be built.
 */
class Solution {
    /**
     * @param {!SolutionConfig} config
     */
    constructor(config) {
        this.config = config;
        this.listeners = {};
        this.inStreamHelpers = {};
        this.outStreamHelpers = {};
        this.needToInitializeWasm = true;
        this.needToInitializeGraph = true;
        this.runningPromise = Promise.resolve();
        this.locateFile = (config && config.locateFile) || defaultLocateFile;
        this.packagePath = getPackagePath();
    }
    /**
     * @return {!Promise<void>}
     */
    close() {
        if (this.solutionWasm) {
            this.solutionWasm.delete();
        }
        return Promise.resolve();
    }
    /**
     * Loads all of the dependent WASM files. This is heavy, so we make sure to
     * only do this once.
     * @private
     * @return {!Promise<void>}
     */
    tryToInitializeWasm() {
        return tslib_1.__awaiter(this, void 0, void 0, function* () {
            if (!this.needToInitializeWasm) {
                return;
            }
            // Set up the file loader for both external loaders.
            setWindow(API_EXPORT_NAME, { locateFile: this.locateFile });
            setWindow(PACK_EXPORT_NAME, { locateFile: this.locateFile });
            /** @type {!ReadonlyArray<!FileEntry>} */
            const files = (this.config.files) || [];
            /** @type {boolean} */
            const supportsSimd = yield isSupportedSimd();
            yield Promise.all(files.map((/**
             * @param {!FileEntry} x
             * @return {!Promise<void>}
             */
            x => {
                /** @type {boolean} */
                const doLoad = (x.simd === undefined) || (x.simd && supportsSimd) ||
                    (!x.simd && !supportsSimd);
                if (doLoad) {
                    return loadScript(this.locateFile(x.url, this.packagePath));
                }
                return Promise.resolve();
            })));
            // The variables we set earlier will not be mutated, each according to its
            // related loader.
            /** @type {function(!tsickle_solutions_wasm_1.PackLoader): !Promise<!tsickle_solutions_wasm_1.MediapipeWasm>} */
            const apiFn = (/** @type {function(!tsickle_solutions_wasm_1.PackLoader): !Promise<!tsickle_solutions_wasm_1.MediapipeWasm>} */ (getWindow(API_EXPORT_NAME)));
            /** @type {!tsickle_solutions_wasm_1.PackLoader} */
            const packFn = (/** @type {!tsickle_solutions_wasm_1.PackLoader} */ (getWindow(PACK_EXPORT_NAME)));
            // Now that everything is loaded and mutated, we can finally initialize
            // the WASM loader with the pack loader. The WASM loader will wait until
            // all of the files in the pack loader are complete before resolving its
            // Promise.
            this.wasm = yield apiFn(packFn);
            // TODO(mhays): Developer should be able to explicitly load/unload a
            // solution to prevent stealing all of the WebGL resources (e.g., Chrome
            // may limit the number of WebGL contexts by domain).
            // This is the single canvas that will be used for all texture transfer
            // to and from the back end.
            this.glCanvas = document.createElement('canvas');
            this.wasm.canvas = this.glCanvas;
            this.wasm.createContext(this.glCanvas, /* useWebGl= */ true, 
            /* setInModule= */ true, {});
            // The graph only needs to be loaded once into the solution, but the WASM
            // might re-initialize the solution, and that will specifically happen
            // during wasm.ProcessFrame.
            this.solutionWasm = new this.wasm.SolutionWasm();
            /** @type {!GraphDataImpl} */
            const graphData = new GraphDataImpl(this.config.graph, this.locateFile, this.packagePath);
            yield this.loadGraph(graphData);
            // Listeners modify the graph config internally, so graph listeners are
            // attached only once. Users are free to connect and disconnect callbacks
            // to these listeners as needed.
            if (this.config.listeners) {
                for (const listener of this.config.listeners) {
                    this.addListenerPort(listener);
                }
            }
            this.needToInitializeWasm = false;
        });
    }
    /**
     * Sets the options for the graph, potentially triggering a reinitialize.
     * The triggering is contingent upon the options matching those set up in
     * the solution configuration. If a match is found, initialize is set to run
     * on the next processFrame.
     *
     * We do not create a WASM object here, because it's possible (likely) that
     * WASM has not loaded yet (i.e., the user calls setOptions before calling
     * processFrame / initialize).  We'll do that during initialize when we know
     * it's safe.
     * @param {!OptionList} options
     * @return {void}
     */
    setOptions(options) {
        if (!this.config.options) {
            return;
        }
        /** @type {!Array<!tsickle_solutions_wasm_1.GraphOptionChangeRequest>} */
        const pendingChanges = [];
        for (const option of Object.keys(options)) {
            // Look each option in the option config.
            /** @type {!OptionConfig} */
            const optionConfig = this.config.options[option];
            if (optionConfig) {
                if (optionConfig.graphOptionXref) {
                    /** @type {{valueNumber: number, valueBoolean: boolean}} */
                    const optionValue = {
                        valueNumber: optionConfig.type === OptionType.NUMBER ?
                            (/** @type {number} */ (options[option])) :
                            0.0,
                        valueBoolean: optionConfig.type === OptionType.BOOL ?
                            (/** @type {boolean} */ (options[option])) :
                            false
                    };
                    /** @type {{calculatorName: string, calculatorIndex: number}} */
                    const defaults = { calculatorName: '', calculatorIndex: 0 };
                    // Combine the xref with the value. This is what the WASM will be
                    // expecting.
                    /** @type {*} */
                    const request = Object.assign(Object.assign(Object.assign({}, defaults), optionConfig.graphOptionXref), optionValue);
                    pendingChanges.push(request);
                }
            }
        }
        if (pendingChanges.length !== 0) {
            this.needToInitializeGraph = true;
            this.pendingChanges = pendingChanges;
        }
    }
    /**
     * Initializes the graph is it has not been loaded, or has been triggered to
     * reload (setOptions was called while the graph was running).
     * @private
     * @return {!Promise<void>}
     */
    tryToInitializeGraph() {
        return tslib_1.__awaiter(this, void 0, void 0, function* () {
            if (!this.needToInitializeGraph) {
                return;
            }
            // Move the video frame into the texture.
            /** @type {(null|!WebGL2RenderingContext)} */
            const gl = this.glCanvas.getContext('webgl2');
            if (!gl) {
                alert('Failed to create WebGL canvas context when passing video frame.');
                return;
            }
            this.gl = gl;
            // Changing options on the graph will mutate the graph config.
            if (this.pendingChanges) {
                /** @type {!tsickle_solutions_wasm_1.GraphOptionChangeRequestList} */
                const changeList = new this.wasm.GraphOptionChangeRequestList();
                for (const change of this.pendingChanges) {
                    changeList.push_back(change);
                }
                this.solutionWasm.changeOptions(changeList);
                changeList.delete();
                this.pendingChanges = undefined;
            }
            this.needToInitializeGraph = false;
        });
    }
    /**
     * @return {!Promise<void>}
     */
    initialize() {
        return tslib_1.__awaiter(this, void 0, void 0, function* () {
            yield this.tryToInitializeWasm();
            yield this.tryToInitializeGraph();
        });
    }
    /**
     * @private
     * @param {!GraphData} graph
     * @return {!Promise<void>}
     */
    loadGraph(graph) {
        return tslib_1.__awaiter(this, void 0, void 0, function* () {
            /** @type {!ArrayBuffer} */
            const graphData = yield graph.toArrayBuffer();
            this.solutionWasm.loadGraph(graphData);
        });
    }
    /**
     * Uses the 'type' field of the input config to turn the `unknown` input into
     * something usable by the back end.  In general, we are using emscripten
     * embind's value_object to effect the transfer, so the data must be simply
     * marshalable.
     * @private
     * @param {!InputConfig} config
     * @param {*} input
     * @return {!Object}
     */
    prepareInput(config, input) {
        switch (config.type) {
            case 'video':
                /** @type {!tsickle_stream_helpers_2.ImageStreamHelper} */
                let helper = (/** @type {!tsickle_stream_helpers_2.ImageStreamHelper} */ (this.inStreamHelpers[config.stream]));
                if (!helper) {
                    helper = new streamHelpers.ImageStreamHelper(this.wasm, this.gl);
                    this.inStreamHelpers[config.stream] = helper;
                }
                return helper.encode((/** @type {(!HTMLVideoElement|!HTMLCanvasElement|!HTMLImageElement)} */ (input)));
            default:
                return {};
        }
    }
    /**
     * Allows the user to send one or more inputs to the graph. The graph will
     * run and potentially produce outputs for any attached listeners. The data
     * that a javascript user supplies needs to be conditioned for use by
     * mediapipe, and those details are hidden away (see `prepareInput`).
     * @param {!InputMap} inputs
     * @param {(undefined|number)=} at Timestamp in ms, if undefined, we'll use performance.now().
     * @return {!Promise<void>}
     */
    send(inputs, at) {
        return tslib_1.__awaiter(this, void 0, void 0, function* () {
            if (!this.config.inputs) {
                return;
            }
            /** @type {number} */
            const timestamp = 1000 * (at || performance.now());
            yield this.runningPromise;
            yield this.initialize();
            /** @type {!tsickle_solutions_wasm_1.PacketDataList} */
            const dataValueList = new this.wasm.PacketDataList();
            for (const key of Object.keys(inputs)) {
                /** @type {!InputConfig} */
                const inputConfig = this.config.inputs[key];
                if (inputConfig) {
                    // We only support texture2d at the moment, but once we support more,
                    // this will be a switch statement so that we call the correct push
                    // method.
                    /** @type {!Object} */
                    const input = this.prepareInput(inputConfig, inputs[key]);
                    /** @type {string} */
                    const stream = inputConfig.stream;
                    dataValueList.pushTexture2d((/** @type {!tsickle_solutions_wasm_1.Texture2dData} */ (Object.assign(Object.assign({}, input), { stream, timestamp }))));
                }
            }
            this.solutionWasm.send(dataValueList);
            dataValueList.delete();
        });
    }
    /**
     * @private
     * @param {!WasmResultMap} results
     * @param {(undefined|!OutputConfigMap)=} configMap
     * @return {!ResultMap}
     */
    transformOutputs(results, configMap) {
        if (!configMap) {
            return results;
        }
        /** @type {!ResultMap} */
        const outputs = {};
        for (const key of Object.keys(configMap)) {
            /** @type {(string|!OutputConfigBase)} */
            const config = configMap[key];
            if (typeof config === 'string') {
                outputs[key] = this.extractWasmResult(key, results[config]);
            }
            else {
                /** @type {!tsickle_solutions_wasm_1.ResultWasm} */
                const result = results[config.stream];
                if (result === undefined) {
                    continue;
                }
                if (config.type === 'detection_list') {
                    outputs[key] = decodeDetectionList(result);
                }
                else if (config.type === 'landmarks') {
                    /** @type {(undefined|!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.NormalizedLandmark>)} */
                    const value = result.getLandmarks();
                    outputs[key] = value ? decodeLandmarks(value) : undefined;
                }
                else if (config.type === 'landmarks_list') {
                    /** @type {(undefined|!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.NormalizedLandmark>>)} */
                    const value = result.getLandmarksList();
                    outputs[key] = value ? decodeLandmarksList(value) : undefined;
                }
                else if (config.type === 'rect_list') {
                    /** @type {(undefined|!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.NormalizedRect>)} */
                    const value = result.getRectList();
                    outputs[key] = value ? decodeRectList(value) : undefined;
                }
                else if (config.type === 'classifications_list') {
                    /** @type {(undefined|!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.Classification>>)} */
                    const value = result.getClassificationsList();
                    outputs[key] = value ? decodeClassificationsList(value) : undefined;
                }
                else if (config.type === 'object_detection_list') {
                    /** @type {(undefined|!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.ObjectDetection>)} */
                    const value = result.getObjectDetectionList();
                    outputs[key] = value ? decodeObjectDetectionList(value) : undefined;
                }
                else if (config.type === 'texture') {
                    /** @type {!tsickle_stream_helpers_2.ImageStreamHelper} */
                    let helper = (/** @type {!tsickle_stream_helpers_2.ImageStreamHelper} */ (this.outStreamHelpers[key]));
                    if (!helper) {
                        helper = new streamHelpers.ImageStreamHelper(this.wasm, this.gl);
                        this.outStreamHelpers[key] = helper;
                    }
                    outputs[key] = helper.decode((/** @type {!tsickle_solutions_wasm_1.Texture2dData} */ (result.getTexture2d())));
                }
                else {
                    throw new Error(`Unknown output config type: '${config.type}'`);
                }
                if (config.transform && outputs[key]) {
                    outputs[key] = config.transform(outputs[key]);
                }
            }
        }
        return outputs;
    }
    /**
     * Extracts the result from a collection of results. This tries to autodetect
     * the type and does no transformation. Arrays will have the slightly more
     * awkward emscripten accessors.
     * \@TODO(mhays) Deprecate this method and always specify the type.
     * @private
     * @param {string} stream
     * @param {!tsickle_solutions_wasm_1.ResultWasm} result
     * @return {*}
     */
    extractWasmResult(stream, result) {
        if (result.isNumber()) {
            return result.getNumber();
        }
        if (result.isRect()) {
            return result.getRect();
        }
        if (result.isLandmarks()) {
            return result.getLandmarks();
        }
        if (result.isLandmarksList()) {
            return result.getLandmarksList();
        }
        if (result.isClassificationsList()) {
            return result.getClassificationsList();
        }
        if (result.isObjectDetectionList()) {
            return result.getObjectDetectionList();
        }
        if (result.isTexture2d()) {
            /** @type {!tsickle_stream_helpers_2.ImageStreamHelper} */
            let helper = (/** @type {!tsickle_stream_helpers_2.ImageStreamHelper} */ (this.outStreamHelpers[stream]));
            if (!helper) {
                helper = new streamHelpers.ImageStreamHelper(this.wasm, this.gl);
                this.outStreamHelpers[stream] = helper;
            }
            return helper.decode((/** @type {!tsickle_solutions_wasm_1.Texture2dData} */ (result.getTexture2d())));
        }
        return undefined;
    }
    /**
     * Attaches a listener that will be called when the graph produces
     * compatible packets on the named stream.
     * @private
     * @param {!ListenerPort} port
     * @return {void}
     */
    addListenerPort(port) {
        /** @type {string} */
        const portName = port.name || DEFAULT_PORT_NAME;
        /** @type {!Array<string>} */
        const wantsCopy = [...port.wants];
        /** @type {!tsickle_solutions_wasm_1.SimpleVector<string>} */
        const wantsVector = new this.wasm.StringList();
        for (const want of port.wants) {
            wantsVector.push_back(want);
        }
        /** @type {{onResults: function(!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.ResultWasm>): !Promise<void>}} */
        const wasmListener = {
            onResults: (/**
             * @param {!tsickle_solutions_wasm_1.ReadOnlySimpleVector<!tsickle_solutions_wasm_1.ResultWasm>} values
             * @return {!Promise<void>}
             */
            (values) => tslib_1.__awaiter(this, void 0, void 0, function* () {
                /** @type {!WasmResultMap} */
                const wasmResults = {};
                for (let index = 0; index < port.wants.length; ++index) {
                    wasmResults[wantsCopy[index]] = values.get(index);
                }
                /** @type {!ResultMap} */
                const results = this.transformOutputs(wasmResults, port.outs);
                /** @type {(undefined|function(!ResultMap): (void|!Promise<void>))} */
                const listener = this.listeners[portName];
                if (listener) {
                    yield this.runningPromise;
                    /** @type {(void|!Promise<void>)} */
                    const result = listener(results);
                    if (result) {
                        this.runningPromise = result;
                        return result;
                    }
                }
            }))
        };
        /** @type {!tsickle_solutions_wasm_1.PacketListener} */
        const packetListener = this.wasm.PacketListener.implement(wasmListener);
        this.solutionWasm.attachMultiListener(wantsVector, packetListener);
        wantsVector.delete();
    }
    /**
     * Use this to connect up to one listener per port name. If port is undefined,
     * which should be the case if there is only one, then the default port is
     * used.
     * @param {function(!ResultMap): (void|!Promise<void>)} listener
     * @param {(undefined|string)=} port
     * @return {void}
     */
    onResults(listener, port) {
        port = port || DEFAULT_PORT_NAME;
        this.listeners[port] = listener;
    }
}
exports.Solution = Solution;
/* istanbul ignore if */
if (false) {
    /**
     * @type {string}
     * @private
     */
    Solution.prototype.packagePath;
    /**
     * @type {function(string, string): string}
     * @private
     */
    Solution.prototype.locateFile;
    /**
     * @type {!ListenerMap}
     * @private
     */
    Solution.prototype.listeners;
    /**
     * @type {!tsickle_stream_helpers_2.StreamHelperMap}
     * @private
     */
    Solution.prototype.inStreamHelpers;
    /**
     * @type {!tsickle_stream_helpers_2.StreamHelperMap}
     * @private
     */
    Solution.prototype.outStreamHelpers;
    /**
     * @type {!HTMLCanvasElement}
     * @private
     */
    Solution.prototype.glCanvas;
    /**
     * @type {!WebGL2RenderingContext}
     * @private
     */
    Solution.prototype.gl;
    /**
     * @type {!tsickle_solutions_wasm_1.MediapipeWasm}
     * @private
     */
    Solution.prototype.wasm;
    /**
     * @type {!tsickle_solutions_wasm_1.SolutionWasm}
     * @private
     */
    Solution.prototype.solutionWasm;
    /**
     * @type {(undefined|!Array<!tsickle_solutions_wasm_1.GraphOptionChangeRequest>)}
     * @private
     */
    Solution.prototype.pendingChanges;
    /**
     * @type {boolean}
     * @private
     */
    Solution.prototype.needToInitializeWasm;
    /**
     * @type {boolean}
     * @private
     */
    Solution.prototype.needToInitializeGraph;
    /**
     * @type {!Promise<void>}
     * @private
     */
    Solution.prototype.runningPromise;
    /**
     * @type {!SolutionConfig}
     * @private
     */
    Solution.prototype.config;
}
goog.exportSymbol('Solution', Solution);
goog.exportSymbol('OptionType', OptionType);
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoic29sdXRpb25zX2FwaS5qcyIsInNvdXJjZVJvb3QiOiIiLCJzb3VyY2VzIjpbIi4uLy4uLy4uLy4uLy4uLy4uLy4uL3RoaXJkX3BhcnR5L21lZGlhcGlwZS93ZWIvc29sdXRpb25zL3NvbHV0aW9uc19hcGkudHMiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6Ijs7Ozs7Ozs7OztBQUNBLGlHQUFrRDs7Ozs7Ozs7QUFRbEQsZ0NBQThEOzs7OztBQUs5RCx1QkFBMEQ7Ozs7O0FBSzFELDJCQUFrRDs7Ozs7QUFLbEQsd0JBb0JDOzs7Ozs7OztJQWhCQyxnQ0FBNEI7Ozs7Ozs7O0lBUTVCLDhCQUFrQzs7Ozs7OztJQU9sQyxvQ0FBb0M7Ozs7OztBQU10QyxzQkFBd0M7Ozs7O0FBS3hDLGlDQUtDOzs7OztJQUpDLCtCQUFVOztJQUNWLCtCQUFVOztJQUNWLCtCQUFVOztJQUNWLHdDQUFvQjs7Ozs7O0FBTXRCLCtCQUEwRDs7Ozs7QUFLMUQsbUNBQWtFOzs7OztBQUtsRSw2QkFvQkM7Ozs7Ozs7O0lBaEJDLCtCQUFjOzs7OztJQUtkLCtCQUFjOzs7OztJQUtkLCtCQUFlOzs7OztJQUtmLHFDQUFxQjs7Ozs7O0FBTXZCLDJCQUFrRDs7Ozs7QUFLbEQsK0JBQTBEOzs7OztBQUsxRCxzQkFJQzs7Ozs7SUFIQyxvQkFBVTs7SUFDVixvQkFBVTs7SUFDVix3QkFBYzs7Ozs7O0FBTWhCLHNCQUlDOzs7OztJQUhDLG9CQUFVOztJQUNWLG9CQUFVOztJQUNWLG9CQUFVOzs7Ozs7OztBQVFaLHVCQUlDOzs7OztJQUhDLHNCQUFXOztJQUNYLDJCQUFpQjs7SUFDakIsMkJBQWlCOzs7Ozs7QUFNbkIscUJBQXNDOzs7OztBQUt0Qyw4QkFhQzs7Ozs7Ozs7SUFUQyw2QkFBVzs7Ozs7SUFJWCxvQ0FBd0I7Ozs7O0lBSXhCLHFDQUFtQjs7Ozs7O0FBTXJCLDRCQUFvRDs7Ozs7QUFLcEQsa0JBQWdEOzs7Ozs7QUFNaEQsd0JBRUM7Ozs7Ozs7QUFNRCw0QkFFQzs7Ozs7Ozs7QUFPRCwwQkFBdUU7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SUFBSSxDQUFDLEdBQUcsUUFBUSxDQUFDO0lBQ2xDLENBQUM7Q0FDRjtBQXJYRCw0QkFxWEM7Ozs7Ozs7SUFwWEMsK0JBQXFDOzs7OztJQUNyQyw4QkFBMkM7Ozs7O0lBQzNDLDZCQUE2Qzs7Ozs7SUFDN0MsbUNBQXFFOzs7OztJQUNyRSxvQ0FBc0U7Ozs7O0lBR3RFLDRCQUFxQzs7Ozs7SUFDckMsc0JBQW9DOzs7OztJQUNwQyx3QkFBMkM7Ozs7O0lBQzNDLGdDQUFrRDs7Ozs7SUFHbEQsa0NBQWtFOzs7OztJQUVsRSx3Q0FBb0M7Ozs7O0lBQ3BDLHlDQUFxQzs7Ozs7SUFFckMsa0NBQTJDOzs7OztJQUUvQiwwQkFBdUM7O0FBa1dyRCxJQUFJLENBQUMsWUFBWSxDQUFDLFVBQVUsRUFBRSxRQUFRLENBQUMsQ0FBQztBQUN4QyxJQUFJLENBQUMsWUFBWSxDQUFDLFlBQVksRUFBRSxVQUFVLENBQUMsQ0FBQyIsInNvdXJjZXNDb250ZW50IjpbImltcG9ydCAqIGFzIHNvbHV0aW9uc1dhc20gZnJvbSAnLi9zb2x1dGlvbnNfd2FzbSc7XG5pbXBvcnQgKiBhcyBzdHJlYW1IZWxwZXJzIGZyb20gJy4vc3RyZWFtX2hlbHBlcnMnO1xuXG4vKipcbiAqIFJlcHJlc2VudHMgcGFpcnMgb2YgKHN0YXJ0LGVuZCkgaW5kZXhlcyBzbyB0aGF0IHdlIGNhbiBjb25uZWN0IGxhbmRtYXJrc1xuICogd2l0aCBsaW5lcyB0byBwcm92aWRlIGEgc2tlbGV0b24gd2hlbiB3ZSBkcmF3IHRoZSBwb2ludHMuIFRoZXNlIGFyZSBkZWZpbmVkXG4gKiB3aXRoaW4gZWFjaCBzcGVjaWZpYyBzb2x1dGlvbiwgYnV0IGEgY29tbW9uIGRlZmluaXRpb24gaXMgaGVsZCBoZXJlIGZvclxuICogY29udmVuaWVuY2UuXG4gKi9cbmV4cG9ydCB0eXBlIExhbmRtYXJrQ29ubmVjdGlvbkFycmF5ID0gQXJyYXk8W251bWJlciwgbnVtYmVyXT47XG5cbi8qKlxuICogUmVwcmVzZW50cyBhIG5vcm1hbGl6ZWQgcmVjdGFuZ2xlIHRoYXQgY2FuIGJlIHBhc3NlZCB0byBhIFN0cmVhbUxpc3RlbmVyLlxuICovXG5leHBvcnQgdHlwZSBOb3JtYWxpemVkUmVjdCA9IHNvbHV0aW9uc1dhc20uTm9ybWFsaXplZFJlY3Q7XG5cbi8qKlxuICogUmVwcmVzZW50cyBhIGxpc3Qgb2Ygbm9ybWFsaXplZCByZWN0YW5nbGVzLlxuICovXG5leHBvcnQgdHlwZSBOb3JtYWxpemVkUmVjdExpc3QgPSBOb3JtYWxpemVkUmVjdFtdO1xuXG4vKipcbiAqIFJlcHJlc2VudHMgYSBkZXRlY3Rpb24uXG4gKi9cbmV4cG9ydCBpbnRlcmZhY2UgRGV0ZWN0aW9uIHtcbiAgLyoqXG4gICAqIFRoZSBib3VuZGluZyBib3ggYXJvdW5kIHRoZSBkZXRlY3Rpb24uXG4gICAqL1xuICBib3VuZGluZ0JveDogTm9ybWFsaXplZFJlY3Q7XG5cbiAgLyoqXG4gICAqIFRoZSBhc3NvY2lhdGVkIGxhbmRtYXJrcy5cbiAgICpcbiAgICogTWF5IGJlIGVtcHR5LiBUaGlzIGlzIHR5cGljYWxseSBmaWxsZWQgZm9yIGUuZy4gZmFjZSwgaGFuZHMgb3IgcG9zZVxuICAgKiBkZXRlY3Rpb24gdXNlIGNhc2VzLlxuICAgKi9cbiAgbGFuZG1hcmtzOiBOb3JtYWxpemVkTGFuZG1hcmtMaXN0O1xuXG4gIC8qKlxuICAgKiBUaGUgYXNzb2NpYXRlZCBjbGFzc2lmaWNhdGlvbnMuXG4gICAqXG4gICAqIE1heSBiZSBlbXB0eS4gVGhpcyBpcyB0eXBpY2FsbHkgZmlsbGVkIGZvciBlLmcuIG9iamVjdCBkZXRlY3Rpb24gdXNlIGNhc2VzLlxuICAgKi9cbiAgY2xhc3NpZmljYXRpb25zOiBDbGFzc2lmaWNhdGlvbkxpc3Q7XG59XG5cbi8qKlxuICogUmVwcmVzZW50cyBhIGxpc3Qgb2YgZGV0ZWN0aW9ucy5cbiAqL1xuZXhwb3J0IHR5cGUgRGV0ZWN0aW9uTGlzdCA9IERldGVjdGlvbltdO1xuXG4vKipcbiAqIFJlcHJlc2VudHMgYSBkZXRlY3RlZCBsYW5kbWFyay5cbiAqL1xuZXhwb3J0IGludGVyZmFjZSBOb3JtYWxpemVkTGFuZG1hcmsge1xuICB4OiBudW1iZXI7XG4gIHk6IG51bWJlcjtcbiAgejogbnVtYmVyO1xuICB2aXNpYmlsaXR5PzogbnVtYmVyO1xufVxuXG4vKipcbiAqIFJlcHJlc2VudHMgYSBsaXN0IG9mIGxhbmRtYXJrcy5cbiAqL1xuZXhwb3J0IHR5cGUgTm9ybWFsaXplZExhbmRtYXJrTGlzdCA9IE5vcm1hbGl6ZWRMYW5kbWFya1tdO1xuXG4vKipcbiAqIFJlcHJlc2VudHMgYSBsaXN0IG9mIGxpc3RzIG9mIGxhbmRtYXJrcy5cbiAqL1xuZXhwb3J0IHR5cGUgTm9ybWFsaXplZExhbmRtYXJrTGlzdExpc3QgPSBOb3JtYWxpemVkTGFuZG1hcmtMaXN0W107XG5cbi8qKlxuICogQSBjbGFzcyBsYWJlbCB3aXRoIHNjb3JlLlxuICovXG5leHBvcnQgaW50ZXJmYWNlIENsYXNzaWZpY2F0aW9uIHtcbiAgLyoqXG4gICAqIFRoZSBpbmRleCBvZiB0aGUgY2xhc3MgaW4gdGhlIGNvcnJlc3BvbmRpbmcgbGFiZWwgbWFwLlxuICAgKi9cbiAgaW5kZXg6IG51bWJlcjtcblxuICAvKipcbiAgICogVGhlIHByb2JhYmlsaXR5IHNjb3JlIGZvciB0aGlzIGNsYXNzLlxuICAgKi9cbiAgc2NvcmU6IG51bWJlcjtcblxuICAvKipcbiAgICogTGFiZWwgb3IgbmFtZSBvZiB0aGUgY2xhc3MuXG4gICAqL1xuICBsYWJlbD86IHN0cmluZztcblxuICAvKipcbiAgICogRGlzcGxheSBuYW1lIG9mIHRoZSBjbGFzcywgZm9yIHZpc3VhbGl6YXRpb24gcHVycG9zZXMuXG4gICAqL1xuICBkaXNwbGF5TmFtZT86IHN0cmluZztcbn1cblxuLyoqXG4gKiBBIGxpc3Qgb2YgY2xhc3NpZmljYXRpb25zLlxuICovXG5leHBvcnQgdHlwZSBDbGFzc2lmaWNhdGlvbkxpc3QgPSBDbGFzc2lmaWNhdGlvbltdO1xuXG4vKipcbiAqIE11bHRpcGxlIGxpc3RzIG9mIGNsYXNzaWZpY2F0aW9ucy5cbiAqL1xuZXhwb3J0IHR5cGUgQ2xhc3NpZmljYXRpb25MaXN0TGlzdCA9IENsYXNzaWZpY2F0aW9uTGlzdFtdO1xuXG4vKipcbiAqIFJlcHJlc2VudHMgYSBzaW5nbGUgbm9ybWFsaXplZCBsYW5kbWFyaywgd2l0aCBkZXB0aC5cbiAqL1xuZXhwb3J0IGludGVyZmFjZSBQb2ludDJEIHtcbiAgeDogbnVtYmVyO1xuICB5OiBudW1iZXI7XG4gIGRlcHRoOiBudW1iZXI7XG59XG5cbi8qKlxuICogUmVwcmVzZW50cyBhIHBvaW50IGluIDNEIHNwYWNlLlxuICovXG5leHBvcnQgaW50ZXJmYWNlIFBvaW50M0Qge1xuICB4OiBudW1iZXI7XG4gIHk6IG51bWJlcjtcbiAgejogbnVtYmVyO1xufVxuXG4vKipcbiAqIFJlcHJlc2VudHMgYSBwYXJ0aWN1bGFyIGtleXBvaW50IGZyb20gYW4gb2JqZWN0IGRldGVjdGlvbi4gVGhlIGlkIGRlbm90ZXNcbiAqIHdoaWNoIGtleXBvaW50IHRoaXMgaXMgKGNvbnNpc3RlbnQgb3ZlciB0aW1lKSwgYW5kIHRoZW4gdHdvIHJlcHJlc2VudGF0aW9uc1xuICogYXJlIGluY2x1ZGVkIGZvciB0aGlzIHBvaW50LCBhIDJEIG5vcm1hbGl6ZWQgdmVyc2lvbiwgYW5kIGEgM0QgdmVyc2lvbi5cbiAqL1xuZXhwb3J0IGludGVyZmFjZSBLZXlwb2ludCB7XG4gIGlkOiBudW1iZXI7XG4gIHBvaW50M2Q6IFBvaW50M0Q7XG4gIHBvaW50MmQ6IFBvaW50MkQ7XG59XG5cbi8qKlxuICogTGlzdCBvZiBrZXlwb2ludHMuXG4gKi9cbmV4cG9ydCB0eXBlIEtleXBvaW50TGlzdCA9IEtleXBvaW50W107XG5cbi8qKlxuICogVGhlIHJlbGV2YW50IGluZm9ybWF0aW9uIGFib3V0IGEgcGFydGljdWxhciBvYmplY3QgZGV0ZWN0aW9uLlxuICovXG5leHBvcnQgaW50ZXJmYWNlIE9iamVjdERldGVjdGlvbiB7XG4gIC8qKlxuICAgKiBUaGUgdW5pcXVlIG9iamVjdCBpbnN0YW5jZSBpZGVudGlmaWVyLlxuICAgKi9cbiAgaWQ6IG51bWJlcjtcbiAgLyoqXG4gICAqIFRoZSBsaXN0IG9mIGtleXBvaW50cyBmb3IgdGhpcyBkZXRlY3Rpb24gKDggZm9yIGEgM0QgYm91bmRpbmcgYm94KS5cbiAgICovXG4gIGtleXBvaW50czogS2V5cG9pbnRMaXN0O1xuICAvKipcbiAgICogVGhlIHZpc2liaWxpdHkgb2YgdGhpcyBvYmplY3QgZGV0ZWN0aW9uIGluIGEgZnJhbWUuXG4gICAqL1xuICB2aXNpYmlsaXR5OiBudW1iZXI7XG59XG5cbi8qKlxuICogTGlzdCBvZiBvYmplY3QgZGV0ZWN0aW9ucy5cbiAqL1xuZXhwb3J0IHR5cGUgT2JqZWN0RGV0ZWN0aW9uTGlzdCA9IE9iamVjdERldGVjdGlvbltdO1xuXG4vKipcbiAqIFJlcHJlc2VudHMgYW4gaW1hZ2UgcHJvZHVjZWQgYnkgdGhlIGdyYXBoLlxuICovXG5leHBvcnQgdHlwZSBHcHVCdWZmZXIgPSBzdHJlYW1IZWxwZXJzLkdwdUJ1ZmZlcjtcblxuLyoqXG4gKiBSZXByZXNlbnRzIGEgc2luZ2xlIHJlc3VsdCBvZiBhIG11bHRpIHJlc3VsdCBsaXN0ZW5lci4gVGhlIHNoYXBlIGlzXG4gKiB0cmFuc2Zvcm1lZCBpbnRvIGEgc2hhcGUgdGhhdCB0aGUgZW5kIHVzZXIgY2FuIGNvbnN1bWUuXG4gKi9cbmV4cG9ydCBpbnRlcmZhY2UgUmVzdWx0TWFwIHtcbiAgW2tleTogc3RyaW5nXTogdW5rbm93bjtcbn1cblxuLyoqXG4gKiBSZXByZXNlbnRzIGEgc2luZ2xlIHJlc3VsdCBvZiBhIG11bHRpIHJlc3VsdCBsaXN0ZW5lci4gVGhlIHNoYXBlIGlzIHRoZVxuICogd2FzbSBjbGFzcyBmcm9tIHdoaWNoIHdlIGNhbiBwdWxsIG91dCBkaWZmZXJlbnQga2luZHMgb2YgZGF0YS5cbiAqL1xuZXhwb3J0IGludGVyZmFjZSBXYXNtUmVzdWx0TWFwIHtcbiAgW2tleTogc3RyaW5nXTogc29sdXRpb25zV2FzbS5SZXN1bHRXYXNtO1xufVxuXG4vKipcbiAqIFRoaXMgcmVwcmVzZW50cyB0aGUgY2FsbGJhY2tzIGZyb20gdGhlIGdyYXBoJ3Mgc3RyZWFtIGxpc3RlbmVycy4gSWYgdGhlXG4gKiBsaXN0ZW5lciBuZWVkcyB0byBkbyBhc3luY2hyb25vdXMgd29yaywgcmV0dXJuIGEgUHJvbWlzZSB0byBhbGxvdyB0aGVcbiAqIHNvbHV0aW9uIHRvIHdhaXQuXG4gKi9cbmV4cG9ydCB0eXBlIFJlc3VsdE1hcExpc3RlbmVyID0gKG1hcDogUmVzdWx0TWFwKSA9PiBQcm9taXNlPHZvaWQ+fHZvaWQ7XG5cbi8qKlxuICogT3B0aW9uIGRlc2NyaXB0aW9ucyB0aGF0IGRlc2NyaWJlIHdoZXJlIHRvIGNoYW5nZSBhbiBvcHRpb24gaW4gYSBncmFwaC5cbiAqL1xuZXhwb3J0IHR5cGUgR3JhcGhPcHRpb25YUmVmID0gc29sdXRpb25zV2FzbS5HcmFwaE9wdGlvblhSZWY7XG5cbi8qKlxuICogSWYgeW91ciBmaWxlcyBhcmUgb3JnYW5pemVkIGluIHNvbWUgd2VpcmQgc3RydWN0dXJlLCB5b3UgY2FuIGN1c3RvbWl6ZSBob3dcbiAqIHRoZSBwYXRoIGlzIGNvbnN0cnVjdGVkIGJ5IHNlbmRpbmcgdGhpcyB0byB0aGUgY29uZmlnIG9wdGlvbnMgZm9yIFNvbHV0aW9uLlxuICovXG5leHBvcnQgdHlwZSBGaWxlTG9jYXRvckZuID0gKGZpbGU6IHN0cmluZywgcHJlZml4OiBzdHJpbmcpID0+IHN0cmluZztcblxuLyoqXG4gKiBVc2VycyBvZiB0aGUgc29sdXRpb24gc2V0IG9wdGlvbnMgdXNpbmcgYSBrZXktdmFsdWUgbWFwLiBIb3dldmVyLCBzcGVjaWZpY1xuICogc29sdXRpb25zIHdpbGwgY29uc3RyYWluIHRoZSBvcHRpb25zIHdpdGggYW4gZXhwb3J0ZWQgaW50ZXJmYWNlLlxuICpcbiAqIGUuZy5cbiAqXG4gKiBnZW5lcmFsU29sdXRpb24uc2V0T3B0aW9ucyh7XCJteUtleVwiOiBteVZhbHVlfSkgYmVjb21lc1xuICogc3BlY2lmaWNTb2x1dGlvbi5zZXRPcHRpb25zKHsgbXlLZXk6IG15VmFsdWUgfX1cbiAqXG4gKiBkZWNsYXJlIGludGVyZmFjZSBTcGVjaWZpY1NvbHV0aW9uT3B0aW9uTGlzdCB7XG4gKiAgIG15S2V5OiBudW1iZXJcbiAqIH07XG4gKlxuICogVGhpcyB3b3JrcyBiZWNhdXNlIGBPYmplY3RgcyBpbiBKYXZhU2NyaXB0IGFyZSBqdXN0IGhhc2htYXBzLCBhbmQgd2UgZml0XG4gKiBtZWFuaW5nIG9uIHRvcCBvZiB0aG9zZSBoYXNobWFwcyB3aXRoIG91ciB0eXBlIGRlY2xhcmF0aW9ucy5cbiAqL1xuZXhwb3J0IGludGVyZmFjZSBPcHRpb25MaXN0IHtcbiAgW2tleTogc3RyaW5nXTogdW5rbm93bjtcbn1cblxuLyoqXG4gKiBVc2VkIGluIHRoZSBjb25zdHJ1Y3RvciBvZiBHcmFwaC5cbiAqL1xuZXhwb3J0IGludGVyZmFjZSBHcmFwaCB7XG4gIHVybDogc3RyaW5nO1xufVxuXG4vKipcbiAqIERlc2NyaWJlcyBob3cgdG8gdHVybiB1c2VyIGlucHV0IChTZWUgSW5wdXRNYXApIGludG8gZGF0YSB0aGUgZ3JhcGggY2FuXG4gKiBjb25zdW1lLlxuICovXG5leHBvcnQgaW50ZXJmYWNlIElucHV0Q29uZmlnIHtcbiAgLyoqXG4gICAqIFByb3ZpZGVzIGRpcmVjdGlvbiBvbiBob3cgdG8gaW50ZXJwcmV0IHRoZSBgdmFsdWVgIG9mIHRoZSBJbnB1dE1hcC5cbiAgICovXG4gIHR5cGU6IHN0cmluZztcblxuICAvKipcbiAgICogVGhlIG5hbWUgb2YgdGhlIGlucHV0IHN0cmVhbSBvZiB0aGUgZ3JhcGggdGhhdCB3aWxsIHJlY2VpdmUgdGhlIGlucHV0LlxuICAgKi9cbiAgc3RyZWFtOiBzdHJpbmc7XG59XG5cbi8qKlxuICogVGhlIGZ1bGwgZGVzY3JpcHRpb24gb2YgYWxsIHRoZSBpbnB1dHMgdGhhdCB3aWxsIGJlIGV4cG9zZWQgdG8gdGhlIHVzZXIuIFRoZVxuICoga2V5IGlzIHRoZSBuYW1lIG9mIHRoZSBpbnB1dCBhIHVzZXIgd2lsbCB1c2UuXG4gKi9cbmV4cG9ydCBpbnRlcmZhY2UgSW5wdXRDb25maWdNYXAge1xuICBba2V5OiBzdHJpbmddOiBJbnB1dENvbmZpZztcbn1cblxuLyoqXG4gKiBEZXNjcmliZXMgdGhlIGlucHV0cyB0aGF0IHRoZSB1c2VyIHdpbGwgc2VuZCB0byB0aGUgZ3JhcGguIGB1bmtub3duYCBhbGxvd3NcbiAqIHVzZXJzIHRvIGRvIGFsbCBzb3J0cyBvZiB0aGluZ3MsIGZyb20gdXNpbmcgSFRNTFZpZGVvRWxlbWVudHMgdG8gbnVtYmVycyB0b1xuICogd2hhdGV2ZXIgZWxzZSB3ZSBuZWVkLiBTZWUgYHByZXBhcmVJbnB1dGAgdG8gc2VlIGhvdyB0aGlzIHVua25vd24gaXMgdHVybmVkXG4gKiBpbnRvIHN0cnVjdHVyYWwgZGF0YSB0aGF0IGNhbiBiZSBzZW50IHRvIHRoZSBncmFwaC5cbiAqXG4gKiBUaGUgc3BlY2lmaWMgc29sdXRpb24gc2hvdWxkIHN1cHBseSBhIHN0cm9uZ2VyIGludGVyZmFjZSB0aGF0IHdpbGwgYmUgdXNlZFxuICogaW5zdGVhZCBvZiB0aGlzIGdlbmVyaWMga2V5bWFwLCAgSW4gdGhpcyB3YXksIHRoZSBzb2x1dGlvbiBkZXNpZ25lciBjYW5cbiAqIG5hbWUgdmVyeSBzcGVjaWZpYyBpbnB1dHMgYW5kIGV4YWN0bHkgd2hhdCBpcyBleHBlY3RlZCBpbiB0aGF0IElucHV0TWFwLidcbiAqL1xuZXhwb3J0IGludGVyZmFjZSBJbnB1dE1hcCB7XG4gIFtrZXk6IHN0cmluZ106IHVua25vd247XG59XG5cbi8qKlxuICogV2Ugc3VwcG9ydCBzZXZlcmFsIHdheXMgdG8gZ2V0IGltYWdlIGlucHV0cy5cbiAqL1xuZXhwb3J0IHR5cGUgSW5wdXRJbWFnZSA9IEhUTUxWaWRlb0VsZW1lbnR8SFRNTEltYWdlRWxlbWVudHxIVE1MQ2FudmFzRWxlbWVudDtcblxuLyoqXG4gKiBEZXNjcmliZXMgaG93IHRvIGludGVycHJldCBPcHRpb25Db25maWcuIEBzZWUgR3JhcGhPcHRpb25DaGFuZ2VSZXF1ZXN0LlxuICogZS5nLiwgQSBOVU1CRVIgdGVsbHMgdXMgdG8gZmlsbCBvdXQgdmFsdWVOdW1iZXIgKHdoaWNoIGdldHMgaW50ZXJwcmV0ZWQgYXNcbiAqIGEgZG91YmxlIG9uIHRoZSBDKysgc2lkZSkuIFdlIHdpbGwgYWRkIG90aGVyIHR5cGVzIGhlcmUgYXMgdGhlIG5lZWQgYXJpc2VzLlxuICovXG5leHBvcnQgZW51bSBPcHRpb25UeXBlIHtcbiAgTlVNQkVSLFxuICBCT09MLFxufVxuXG4vKipcbiAqIFVzZWQgdG8gcmVnaXN0ZXIga2V5cyB3aXRoIHRoZSBzb2x1dGlvbi4gUmlnaHQgbm93IGl0IGlzIGxpbWl0ZWQgdG9cbiAqIGdyYXBoT3B0aW9uWHJlZiwgd2hpY2ggaXMgc3BlY2lmaWNhbGx5IGZvciBvcHRpb25zIHRoYXQgd2lsbCBiZSBmZWQgdG8gdGhlXG4gKiBNZWRpYVBpcGUgZ3JhcGgsIGJ1dCB3ZSdsbCBhbHNvIG5lZWQgb3B0aW9ucyB0aGF0IGFyZSBoYW5kbGVkIGJ5IHRoZSBzcGVjaWZpY1xuICogc29sdXRpb24gdG8gZG8gd29yayAoZS5nLiwgc2VsZmllLWZsaXBwaW5nKS5cbiAqL1xuZXhwb3J0IGludGVyZmFjZSBPcHRpb25Db25maWcge1xuICB0eXBlOiBPcHRpb25UeXBlO1xuICBncmFwaE9wdGlvblhyZWY/OiBHcmFwaE9wdGlvblhSZWY7XG59XG5cbi8qKlxuICogT3B0aW9uYWxseSBzcGVjaWZpZWQgaW4gdGhlIG91dHB1dCB0byB0cmFuc2Zvcm0gdGhlIG91dHB1dC5cbiAqL1xuZXhwb3J0IHR5cGUgVHJhbnNmb3JtZXI8VCwgTz4gPSAoeDogVCkgPT4gTztcblxuLyoqXG4gKiBPdXRwdXQgY29uZmlndXJhdGlvbiB3aXRob3V0IGEgdHJhbnNmb3JtIGZ1bmN0aW9uLlxuICovXG5leHBvcnQgaW50ZXJmYWNlIE91dHB1dENvbmZpZ0Jhc2Uge1xuICBzdHJlYW06IHN0cmluZztcbiAgdHlwZTogJ2xhbmRtYXJrcyd8J2xhbmRtYXJrc19saXN0J3wnY2xhc3NpZmljYXRpb25zX2xpc3QnfFxuICAgICAgJ29iamVjdF9kZXRlY3Rpb25fbGlzdCd8J3JlY3RfbGlzdCd8J2RldGVjdGlvbl9saXN0JztcbiAgLy8gSW50ZW50aW9uYWwgYGFueWAgYWxsb3dzIGRlcml2ZWQgY2xhc3NlcyB0byBzdXBwb3J0IGN1c3RvbSB0cmFuc2Zvcm1lcnMuXG4gIC8vIHRzbGludDpkaXNhYmxlLW5leHQtbGluZTpuby1hbnlcbiAgdHJhbnNmb3JtPzogVHJhbnNmb3JtZXI8YW55LCBhbnk+O1xufVxuXG4vKipcbiAqIE91dHB1dCBjb25maWcgd2l0aCB0eXBlIHNhZmV0eSBmb3IgYSBsaXN0IG9mIGxhbmRtYXJrcy5cbiAqL1xuZXhwb3J0IGludGVyZmFjZSBPdXRwdXRDb25maWdMYW5kbWFya0xpc3Q8VD4gZXh0ZW5kcyBPdXRwdXRDb25maWdCYXNlIHtcbiAgdHlwZTogJ2xhbmRtYXJrcyc7XG4gIHRyYW5zZm9ybT86IFRyYW5zZm9ybWVyPE5vcm1hbGl6ZWRMYW5kbWFya0xpc3QsIFQ+O1xufVxuXG4vKipcbiAqIE91dHB1dCBjb25maWcgd2l0aCB0eXBlIHNhZmV0eSBmb3IgYSBsaXN0IG9mIGxhbmRtYXJrIGxpc3RzLlxuICovXG5leHBvcnQgaW50ZXJmYWNlIE91dHB1dENvbmZpZ0xhbmRtYXJrTGlzdExpc3Q8VD4gZXh0ZW5kcyBPdXRwdXRDb25maWdCYXNlIHtcbiAgdHlwZTogJ2xhbmRtYXJrc19saXN0JztcbiAgdHJhbnNmb3JtPzogVHJhbnNmb3JtZXI8Tm9ybWFsaXplZExhbmRtYXJrTGlzdExpc3QsIFQ+O1xufVxuXG4vKipcbiAqIE91dHB1dCBjb25maWcgd2l0aCB0eXBlIHNhZmV0eSBmb3IgYSBsaXN0IG9mIGNsYXNzaWZpY2F0aW9uIGxpc3RzLlxuICovXG5leHBvcnQgaW50ZXJmYWNlIE91dHB1dENvbmZpZ0NsYXNzaWZpY2F0aW9uTGlzdExpc3Q8VD4gZXh0ZW5kc1xuICAgIE91dHB1dENvbmZpZ0Jhc2Uge1xuICB0eXBlOiAnY2xhc3NpZmljYXRpb25zX2xpc3QnO1xuICB0cmFuc2Zvcm0/OiBUcmFuc2Zvcm1lcjxDbGFzc2lmaWNhdGlvbkxpc3RMaXN0LCBUPjtcbn1cblxuLyoqXG4gKiBPdXRwdXQgY29uZmlnIHdpdGggdHlwZSBzYWZldHkgZm9yIGFuIG9iamVjdCBkZXRlY3Rpb24uXG4gKi9cbmV4cG9ydCBpbnRlcmZhY2UgT3V0cHV0Q29uZmlnT2JqZWN0RGV0ZWN0aW9uTGlzdDxUPiBleHRlbmRzIE91dHB1dENvbmZpZ0Jhc2Uge1xuICB0eXBlOiAnb2JqZWN0X2RldGVjdGlvbl9saXN0JztcbiAgdHJhbnNmb3JtPzogVHJhbnNmb3JtZXI8T2JqZWN0RGV0ZWN0aW9uTGlzdCwgVD47XG59XG5cbi8qKlxuICogV2hlbiBzcGVjaWZ5aW5nIG91dHB1dHMsIGNhc3QgdG8gT3V0cHV0Q29uZmlnPFQ+IHRvIGdldCB0eXBlIHNhZmV0eSBiZXR3ZWVuXG4gKiB0aGUgYHR5cGVgIGFuZCB0aGUgYHRyYW5zZm9ybWAuXG4gKi9cbmV4cG9ydCB0eXBlIE91dHB1dENvbmZpZzxUPiA9IE91dHB1dENvbmZpZ0xhbmRtYXJrTGlzdDxUPnxcbiAgICBPdXRwdXRDb25maWdMYW5kbWFya0xpc3RMaXN0PFQ+fE91dHB1dENvbmZpZ0NsYXNzaWZpY2F0aW9uTGlzdExpc3Q8VD58XG4gICAgT3V0cHV0Q29uZmlnT2JqZWN0RGV0ZWN0aW9uTGlzdDxUPjtcblxuLyoqXG4gKiBUcmFuc2xhdGlvbiBtYXAgdG8gZXhwb3NlIHVuZGVybHlpbmcgc3RyZWFtcyB0byB0aGUgdXNlci4gQSBzb2x1dGlvblxuICogZGVzaWduZXIgc2hvdWxkIGRlY2xhcmUgYSB0eXBlIHRvIGNvbnN0cmFpbiB0aGUgc2hhcGUgb2YgdGhlIG91dHB1dHMuXG4gKi9cbmV4cG9ydCBpbnRlcmZhY2UgT3V0cHV0Q29uZmlnTWFwIHtcbiAgLy8gVG9kYXkgdGhlIHZhbHVlIGlzIGEgc3RyaW5nLCB3aGljaCBhbGxvd3MgdXMgdG8gbWFwIGFuIGludGVybmFsIHN0cmVhbSBuYW1lXG4gIC8vIHRvIGEgZnJpZW5kbGllciwgZXh0ZXJuYWxseSB2aXNpYmxlIG5hbWUuIEluIHRoZSBuZWFyIGZ1dHVyZSBpdCB3aWxsIGJlIGFuXG4gIC8vIE9iamVjdCB0byBhbGxvdyBjb21wbGV4IHN0cnVjdHVyZXMsIG9yIGEgZnVuY3Rpb24sIHRvIGFsbG93IGNhbGN1bGF0ZWRcbiAgLy8gdmFsdWVzLlxuICBba2V5OiBzdHJpbmddOiBzdHJpbmd8T3V0cHV0Q29uZmlnQmFzZTtcbn1cblxuLyoqXG4gKiBUaGUgY29sbGVjdGlvbiBvZiBvcHRpb24gY29uZmlndXJhdGlvbiBlbnRyaWVzLCBhcnJhbmdlZCBieSBvcHRpb24gbmFtZS5cbiAqL1xuZXhwb3J0IGludGVyZmFjZSBPcHRpb25Db25maWdNYXAge1xuICBbb3B0aW9uTmFtZTogc3RyaW5nXTogT3B0aW9uQ29uZmlnO1xufVxuXG4vKipcbiAqIEJpbmRzIHRoZSBzdHJlYW1zIHRvIGEgc2V0IG9mIGxpc3RlbmVycyB0aGF0IGNhbiBiZSBzdXJmYWNlZCB0byB0aGVcbiAqIGRldmVsb3Blci5cbiAqL1xuZXhwb3J0IGludGVyZmFjZSBMaXN0ZW5lclBvcnQge1xuICAvKipcbiAgICogSWYgdGhlcmUgaXMgb25seSBvbmUgcmVnaXN0cmF0aW9uLCB0aGUgbmFtZSBjYW4gYmUgb21pdHRlZC4gU2VlIG9uUmVzdWx0c1xuICAgKiBmb3IgbW9yZSBkaXNjdXNzaW9uLlxuICAgKi9cbiAgbmFtZT86IHN0cmluZztcblxuICAvKipcbiAgICogQSBsaXN0IG9mIHN0cmVhbXMgaW4gdGhlIGdyYXBoIHRoYXQgd2lsbCBiZSBjb21iaW5lZCBpbnRvIGEgSGFzaE1hcFxuICAgKiB3aGljaCB3aWxsIGJlIGFzc2VtYmxlZCBhbmQgZGVsaXZlcmVkIHRvIGxpc3RlbmVycyB0aGF0IHRoZSBlbmQgdXNlclxuICAgKiBjYW4gbGlzdGVuIHRvLlxuICAgKi9cbiAgd2FudHM6IHN0cmluZ1tdO1xuXG4gIC8qKlxuICAgKiBBbGxvd3MgdGhlIHJlc3VsdHMgdG8gYmUgdHJhbnNmb3JtZWQgaW50byBsaXRlcmFsbHkgYW55IGtpbmQgb2Ygc2hhcGUuIEl0XG4gICAqIGlzIHVwIHRvIHRoZSBzb2x1dGlvbiBjcmVhdG9yIHRvIGV4cG9zZSB0aGlzIHR5cGUgaW5mb3JtYXRpb24gdGhyb3VnaFxuICAgKiBhbiBvblJlc3VsdHMgc3BlY2lhbGl6YXRpb24uXG4gICAqL1xuICBvdXRzPzogT3V0cHV0Q29uZmlnTWFwO1xuXG4gIC8qKlxuICAgKiBBbGxvd3MgdHJhbnNmb3JtYXRpb24gb2YgYG91dHNgIGJlZm9yZSBoYW5kaW5nIHRoZSByZXN1bHRzIGJhY2sgdG8gdGhlXG4gICAqIGxpc3RlbmVyLiBBbGxvd3MgZmluYWwgY3JlYXRpdmUgY29udHJvbCBvZiB0aGUgc2hhcGUgb2YgdGhlIG91dHB1dC5cbiAgICovXG4gIHRyYW5zZm9ybT86IChpbnB1dDogT3V0cHV0Q29uZmlnTWFwKSA9PiBPdXRwdXRDb25maWdNYXA7XG59XG5cbi8qKlxuICogRGVzY3JpYmVzIGEgZmlsZSB0aGF0IHNob3VsZCBiZSBsb2FkZWQgYnkgdGhlIHNvbHV0aW9uLiBXZSBjYW4gaW5jbHVkZSBmbGFnc1xuICogZm9yIGNoZWNrcyB0aGF0IHNob3VsZCBiZSBkb25lIGJlZm9yZSBsb2FkaW5nIGEgZmlsZS4gQ3VycmVudGx5LCB3ZSBhbGxvd1xuICogYSB1c2VyIHRvIHByb3ZpZGUgc3BlY2lhbCBmaWxlcyB0aGF0IGFyZSBvbmx5IHByb3ZpZGVkIGlmIFNJTUQgaXNcbiAqIHNwZWNpZmljYWxseSBzdXBwb3J0ZWQgb3Igbm90IHN1cHBvcnRlZCBieSB0aGUgYnJvd3NlciAoZG8gbm90IGluY2x1ZGUgdGhlXG4gKiBzaW1kIGZsYWcgaWYgeW91IGRvIG5vdCBjYXJlKS5cbiAqL1xuZXhwb3J0IGludGVyZmFjZSBGaWxlRW50cnkge1xuICBzaW1kPzogYm9vbGVhbjtcbiAgdXJsOiBzdHJpbmc7XG59XG5cbi8qKlxuICogT3B0aW9ucyB0byBjb25maWd1cmUgdGhlIHNvbHV0aW9uLlxuICovXG5leHBvcnQgaW50ZXJmYWNlIFNvbHV0aW9uQ29uZmlnIHtcbiAgLyoqXG4gICAqIFRoZSBwYWNrIGxvYWRlciBhbmQgdGhlIHdhc20gbG9hZGVyIG5lZWQgdG8gYmUgaGVyZS4gQSBmaWxlIGxvYWRlciB3aWxsIGJlXG4gICAqIHByb3ZpZGVkIHRvIHRoZW0sIGFuZCBUaGV5IHdpbGwgZ2V0IGxvYWRlZCBhc3luY2hyb25vdXNseS4gV2Ugd29uJ3RcbiAgICogY29udGludWUgaW5pdGlhbGl6YXRpb24gdW50aWwgdGhleSd2ZSBjb21wbGV0ZWx5IGxvYWRlZCBhbmQgcnVuLlxuICAgKi9cbiAgZmlsZXM6IFJlYWRvbmx5QXJyYXk8RmlsZUVudHJ5PjtcblxuICAvKipcbiAgICogVGhlIGJpbmFyeSBncmFwaC4gQ2FuIHN1cHBvcnQgbXVsdGlwbGUgd2F5cyBvZiBnZXR0aW5nIHRoYXQgZ3JhcGguXG4gICAqL1xuICBncmFwaDogR3JhcGg7XG5cbiAgLyoqXG4gICAqIENvbnRhaW5zIGEgbWFwcGluZyB0aGF0IGFsbG93cyB1cyB0byB0dXJuIHRoZSBpbnB1dHMgYSB1c2VyIHNlbmRzIGFzIHBhcnRcbiAgICogb2YgYHNlbmRgIGludG8gYW4gaW5wdXQgdGhhdCB0aGUgZ3JhcGggY2FuIGNvbnN1bWUuXG4gICAqL1xuICBpbnB1dHM6IElucHV0Q29uZmlnTWFwO1xuXG4gIC8qKlxuICAgKiBTZWUgRmlsZUxvY2F0b3JGbi4gQW55IGZpbGUgbG9hZGluZyBkb25lIG9uIHRoZSB1c2VyJ3MgYmVoYWxmIHdpbGwgdXNlXG4gICAqIGxvY2F0ZUZpbGUgaWYgaXRzIHByb2ZpdmVkLiBUaGlzIGluY2x1ZGVzIFdBU00gYmxvYnMgYW5kIGdyYXBoIHVybHMuXG4gICAqL1xuICBsb2NhdGVGaWxlPzogRmlsZUxvY2F0b3JGbjtcblxuICAvKipcbiAgICogU3BlY2lmaWVzIGhvdyB0byBpbnRlcnByZXQgb3B0aW9ucyBmZWQgdG8gc2V0T3B0aW9ucy5cbiAgICovXG4gIG9wdGlvbnM/OiBPcHRpb25Db25maWdNYXA7XG5cbiAgLyoqXG4gICAqIEF0dGFjaGVzIGxpc3RlbmVycyB0byB0aGUgZ3JhcGguIFRoaXMgaXMgb25seSBmb3IgcmVnaXN0ZXJpbmcgdGhlIHBvcnRzLlxuICAgKiBUbyBhY3R1YWxseSByZWdpc3RlciBhIGNhbGxiYWNrLCB0aGUgZGV2ZWxvcGVyIHdpbGwgY2FsbCBvblJlc3VsdHMgd2l0aFxuICAgKiB0aGUgbmFtZSBvZiB0aGUgcG9ydCB0aGV5IHdhbnQgdG8gdGFwIGludG8uXG4gICAqL1xuICBsaXN0ZW5lcnM/OiBMaXN0ZW5lclBvcnRbXTtcbn1cblxuLyoqXG4gKiBSZXByZXNlbnRzIGEgZ3JhcGggdGhhdCBjYW4gYmUgZmVkIHRvIG1lZGlhcGlwZSBTb2x1dGlvbi5cbiAqL1xuZXhwb3J0IGludGVyZmFjZSBHcmFwaERhdGEge1xuICB0b0FycmF5QnVmZmVyKCk6IFByb21pc2U8QXJyYXlCdWZmZXI+O1xufVxuXG4vLyBUaGVzZSBleHBvcnQgbmFtZXMgY29tZSBmcm9tIHdhc21fY2NfYmluYXJ5IEJVSUxEIHJ1bGVzLiBUaGV5IGJlbG9uZyB0byB0d29cbi8vIGRpZmZlcmVudCBzY3JpcHRzIHRoYXQgYXJlIGxvYWRlZCBpbiBwYXJhbGxlbCAoc2VlIFByb21pc2UuYWxsLCBiZWxvdykuXG4vLyBCZWNhdXNlIHRoZXkgbXV0YXRlIHRoZWlyIHJlc3BlY3RpdmUgdmFyaWFibGVzLCB0aGVyZSBpcyBhIHJhY2UgY29uZGl0aW9uXG4vLyB3aGVyZSB0aGV5IHdpbGwgc3RvbXAgb24gZWFjaCBvdGhlciBpZiB0aGV5IGNob29zZSB0aGUgc2FtZSBuYW1lLiBVc2Vyc1xuLy8gd29uJ3Qgbm9ybWFsbHkgc2VlIHRoaXMgcmFjZSBjb25kaXRpb24gYmVjYXVzZSB0aGV5IHB1dCBzY3JpcHQgdGFncyBpbiB0aGVcbi8vIEhUTUwsIGFuZCBIVE1MIGd1YXJhbnRlZXMgdGhhdCB0aGUgc2NyaXB0cyB3aWxsIGJlIHJ1biBpbiBvcmRlci5cbmNvbnN0IEFQSV9FWFBPUlRfTkFNRSA9ICdjcmVhdGVNZWRpYXBpcGVTb2x1dGlvbnNXYXNtJztcbmNvbnN0IFBBQ0tfRVhQT1JUX05BTUUgPSAnY3JlYXRlTWVkaWFwaXBlU29sdXRpb25zUGFja2VkQXNzZXRzJztcblxuLy8gVXNlZCBpbnRlcm5hbGx5IHRvIHJlcHJlc2VudCBhIHBvcnQgZm9yIHdoaWNoIHRoZSB1c2VyIGRpZCBub3QgZGVmaW5lIGEgbmFtZS5cbmNvbnN0IERFRkFVTFRfUE9SVF9OQU1FID0gJyQnO1xuXG4vKipcbiAqIFNpbXBsZXN0IFdBU00gcHJvZ3JhbSB0byBwZXJmb3JtIGEgU0lNRCBvcGVyYXRpb24uIFdlJ2xsIHRyeSB0byBpbnN0YW50aWF0ZVxuICogaXQgd2l0aCBXZWJBc3NlbWJseS5pbnN0YW50aWF0ZSgpIGFuZCBpZiBpdCBmYWlscywgd2UnbGwga25vdyB0aGVyZSBpcyBub1xuICogU0lNRCBzdXBwb3J0LlxuICpcbiAqIFRvIGdlbmVyYXRlIHRoZSB3ZWIgYXNzZW1ibHksIEkgaW5jbHVkZWQgdGhlIGZvbGxvd2luZyBzY3JpcHQgaW50byBhIHdlYiBwYWdlXG4gKiBodHRwczovL2Nkbi5qc2RlbGl2ci5uZXQvZ2gvQXNzZW1ibHlTY3JpcHQvd2FidC5qcy9pbmRleC5qc1xuICpcbiAqIEFuZCB0aGVuIGZvbGxvd2VkIGl0IHdpdGggYSBiaXQgb2Ygc2NyaXB0OlxuICpcbiAqIGBgYFxuICogYXN5bmMgZnVuY3Rpb24gcnVuKCkge1xuICogdmFyIHdhc20gPSBgKG1vZHVsZVxuICogIChmdW5jXG4gKiAgICAgIGkzMi5jb25zdCAwXG4gKiAgICAgIGk4eDE2LnNwbGF0XG4gKiAgICAgIGRyb3BcbiAqICAgIClcbiAqICApYDtcbiAqICBjb25zdCB3YWJ0ID0gYXdhaXQgV2FidE1vZHVsZSgpO1xuICogIGNvbnN0IHdhdCA9IHdhYnQucGFyc2VXYXQoXCJ0ZXN0XCIsIHdhc20sIHtzaW1kOiB0cnVlfSk7XG4gKiAgY29uc3QgYmluID0gd2F0LnRvQmluYXJ5KHt9KS5idWZmZXI7XG4gKlxuICogIC8vIFRoZSBmb2xsb3dpbmcgbGluZSBwcmludHMgdGhlIGFycmF5IHRoYXQgd2UgY3V0IGFuZCBwYXN0ZSBpbnRvIG91ciBzb3VyY2VcbiAqICAvLyBjb2RlLlxuICogIGNvbnNvbGUubG9nKGJpbi5qb2luKCcsJykpO1xuICpcbiAqICBjb25zdCBhcnIgPSBuZXcgVWludDhBcnJheShiaW4pO1xuICogIGNvbnN0IHdhID0gYXdhaXQgV2ViQXNzZW1ibHkuaW5zdGFudGlhdGUoYXJyKTtcbiAqIH1cbiAqIHJ1bigpO1xuICogYGBgXG4gKi9cbmNvbnN0IFdBU01fU0lNRF9DSEVDSyA9IG5ldyBVaW50OEFycmF5KFtcbiAgMCwgOTcsIDExNSwgMTA5LCAxLCAwLCAwLCAwLCAxLCAgNCwgMSwgICA5NiwgMCwgIDAsIDMsXG4gIDIsIDEsICAwLCAgIDEwLCAgOSwgMSwgNywgMCwgNjUsIDAsIDI1MywgMTUsIDI2LCAxMVxuXSk7XG5cbi8qKlxuICogUmV0dXJucyB0aGUgZGVmYXVsdCBwYXRoIHRvIGEgcmVzb3VyY2UgaWYgdGhlIHVzZXIgZGlkIG5vdCBvdmVybG9hZCB0aGVcbiAqIGxvY2F0ZUZpbGUgcGFyYW1ldGVyIGluIFNvbHV0aW9uQ29uZmlnLlxuICovXG5mdW5jdGlvbiBkZWZhdWx0TG9jYXRlRmlsZShmaWxlOiBzdHJpbmcsIHByZWZpeDogc3RyaW5nKSB7XG4gIHJldHVybiBwcmVmaXggKyBmaWxlO1xufVxuLyoqXG4gKiBTZXRzIGFuIGFyYml0cmFyeSB2YWx1ZSBvbiBgd2luZG93YC4gVGhpcyBoZWxwZXIgZnVuY3Rpb24gZ2V0cyBhcm91bmQgYSBsb3RcbiAqIG9mIHRoZSBjdW1iZXJzb21lIGJvaWxlcnBsYXRlIHJlcXVpcmVkIHRvIHNldCBhbmQgcmV0cmlldmUgYXJiaXRyYXJ5IGRhdGFcbiAqIGZyb20gdGhlIGB3aW5kb3dgIG9iamVjdC5cbiAqL1xuZnVuY3Rpb24gc2V0V2luZG93PFQ+KGtleTogc3RyaW5nLCB2YWx1ZTogVCk6IHZvaWQge1xuICAod2luZG93IGFzIHVua25vd24gYXMge1trZXk6IHN0cmluZ106IFR9KVtrZXldID0gdmFsdWU7XG59XG5cbi8qKlxuICogR2V0cyBhbiBhcmJpdHJhcnkgdmFsdWUgZnJvbSBgd2luZG93YC4gU2VlIGBzZXRXaW5kb3dgLlxuICovXG5mdW5jdGlvbiBnZXRXaW5kb3coa2V5OiBzdHJpbmcpOiB1bmtub3duIHtcbiAgcmV0dXJuICh3aW5kb3cgYXMgdW5rbm93biBhcyB7W2tleTogc3RyaW5nXTogdW5rbm93bn0pW2tleV07XG59XG5cbi8qKlxuICogRHluYW1pY2FsbHkgbG9hZHMgYSBhc2NyaXB0IGludG8gdGhlIGN1cnJlbnQgcGFnZSBhbmQgcmV0dXJucyBhIGBQcm9taXNlYFxuICogdGhhdCByZXNvbHZlcyB3aGVuIGl0cyBsb2FkaW5nIGlzIGNvbXBsZXRlLlxuICovXG5mdW5jdGlvbiBsb2FkU2NyaXB0KHVybDogc3RyaW5nKTogUHJvbWlzZTx2b2lkPiB7XG4gIGNvbnN0IHNjcmlwdCA9IGRvY3VtZW50LmNyZWF0ZUVsZW1lbnQoJ3NjcmlwdCcpO1xuICBzY3JpcHQuc2V0QXR0cmlidXRlKCdzcmMnLCB1cmwpO1xuICBzY3JpcHQuc2V0QXR0cmlidXRlKCdjcm9zc29yaWdpbicsICdhbm9ueW1vdXMnKTtcbiAgZG9jdW1lbnQuYm9keS5hcHBlbmRDaGlsZChzY3JpcHQpO1xuICByZXR1cm4gbmV3IFByb21pc2UoKHJlc29sdmUpID0+IHtcbiAgICBzY3JpcHQuYWRkRXZlbnRMaXN0ZW5lcignbG9hZCcsICgpID0+IHtcbiAgICAgIHJlc29sdmUoKTtcbiAgICB9LCBmYWxzZSk7XG4gIH0pO1xufVxuXG5mdW5jdGlvbiBnZXRQYWNrYWdlUGF0aCgpOiBzdHJpbmcge1xuICBpZiAodHlwZW9mIHdpbmRvdyA9PT0gJ29iamVjdCcpIHtcbiAgICByZXR1cm4gd2luZG93LmxvY2F0aW9uLnBhdGhuYW1lLnRvU3RyaW5nKCkuc3Vic3RyaW5nKFxuICAgICAgICAgICAgICAgMCwgd2luZG93LmxvY2F0aW9uLnBhdGhuYW1lLnRvU3RyaW5nKCkubGFzdEluZGV4T2YoJy8nKSkgK1xuICAgICAgICAnLyc7XG4gIH0gZWxzZSBpZiAodHlwZW9mIGxvY2F0aW9uICE9PSAndW5kZWZpbmVkJykge1xuICAgIC8vIHdvcmtlclxuICAgIHJldHVybiBsb2NhdGlvbi5wYXRobmFtZS50b1N0cmluZygpLnN1YnN0cmluZyhcbiAgICAgICAgICAgICAgIDAsIGxvY2F0aW9uLnBhdGhuYW1lLnRvU3RyaW5nKCkubGFzdEluZGV4T2YoJy8nKSkgK1xuICAgICAgICAnLyc7XG4gIH0gZWxzZSB7XG4gICAgdGhyb3cgbmV3IEVycm9yKFxuICAgICAgICAnc29sdXRpb25zIGNhbiBvbmx5IGJlIGxvYWRlZCBvbiBhIHdlYiBwYWdlIG9yIGluIGEgd2ViIHdvcmtlcicpO1xuICB9XG59XG5cbmZ1bmN0aW9uIGRlY29kZUxhbmRtYXJrcyhsYW5kbWFya3M6IHNvbHV0aW9uc1dhc20uTm9ybWFsaXplZExhbmRtYXJrTGlzdCk6XG4gICAgTm9ybWFsaXplZExhbmRtYXJrTGlzdCB7XG4gIGNvbnN0IG91dHB1dCA9IFtdO1xuICBjb25zdCByZXN1bHRDb3VudCA9IGxhbmRtYXJrcy5zaXplKCk7XG4gIGZvciAobGV0IGluZGV4ID0gMDsgaW5kZXggPCByZXN1bHRDb3VudDsgKytpbmRleCkge1xuICAgIGNvbnN0IGxhbmRtYXJrID0gbGFuZG1hcmtzLmdldChpbmRleCk7XG4gICAgb3V0cHV0LnB1c2goe1xuICAgICAgeDogbGFuZG1hcmsueCxcbiAgICAgIHk6IGxhbmRtYXJrLnksXG4gICAgICB6OiBsYW5kbWFyay56LFxuICAgICAgdmlzaWJpbGl0eTogbGFuZG1hcmsuaGFzVmlzaWJpbGl0eSA/IGxhbmRtYXJrLnZpc2liaWxpdHkgOiB1bmRlZmluZWRcbiAgICB9KTtcbiAgfVxuICByZXR1cm4gb3V0cHV0O1xufVxuXG5mdW5jdGlvbiBkZWNvZGVSZWN0TGlzdChyZWN0czogc29sdXRpb25zV2FzbS5Ob3JtYWxpemVkUmVjdExpc3QpOlxuICAgIE5vcm1hbGl6ZWRSZWN0TGlzdCB7XG4gIGNvbnN0IG91dHB1dCA9IFtdO1xuICBjb25zdCByZXN1bHRDb3VudCA9IHJlY3RzLnNpemUoKTtcbiAgZm9yIChsZXQgaW5kZXggPSAwOyBpbmRleCA8IHJlc3VsdENvdW50OyArK2luZGV4KSB7XG4gICAgY29uc3QgcmVjdCA9IHJlY3RzLmdldChpbmRleCk7XG4gICAgb3V0cHV0LnB1c2gocmVjdCk7XG4gIH1cbiAgcmV0dXJuIG91dHB1dDtcbn1cblxuZnVuY3Rpb24gZGVjb2RlRGV0ZWN0aW9uTGlzdChyZXN1bHRzOiBzb2x1dGlvbnNXYXNtLlJlc3VsdFdhc20pOiBEZXRlY3Rpb25MaXN0IHtcbiAgY29uc3QgcmVjdExpc3QgPSByZXN1bHRzLmdldFJlY3RMaXN0KCkhO1xuICBjb25zdCBsYW5kbWFya3NMaXN0ID0gcmVzdWx0cy5nZXRMYW5kbWFya3NMaXN0KCkhO1xuICBjb25zdCBjbGFzc2lmaWNhdGlvbnNMaXN0ID0gcmVzdWx0cy5nZXRDbGFzc2lmaWNhdGlvbnNMaXN0KCkhO1xuICBjb25zdCBvdXRwdXQgPSBbXTtcbiAgaWYgKHJlY3RMaXN0KSB7XG4gICAgZm9yIChsZXQgaW5kZXggPSAwOyBpbmRleCA8IHJlY3RMaXN0LnNpemUoKTsgKytpbmRleCkge1xuICAgICAgY29uc3QgcmVzdWx0ID0ge1xuICAgICAgICBib3VuZGluZ0JveDogcmVjdExpc3QuZ2V0KGluZGV4KSxcbiAgICAgICAgbGFuZG1hcmtzOiBkZWNvZGVMYW5kbWFya3MobGFuZG1hcmtzTGlzdC5nZXQoaW5kZXgpKSxcbiAgICAgICAgY2xhc3NpZmljYXRpb25zOiBkZWNvZGVDbGFzc2lmaWNhdGlvbnMoY2xhc3NpZmljYXRpb25zTGlzdC5nZXQoaW5kZXgpKSxcbiAgICAgIH07XG4gICAgICBvdXRwdXQucHVzaChyZXN1bHQpO1xuICAgIH1cbiAgfVxuICByZXR1cm4gb3V0cHV0O1xufVxuXG5mdW5jdGlvbiBkZWNvZGVMYW5kbWFya3NMaXN0KFxuICAgIGxhbmRtYXJrczogc29sdXRpb25zV2FzbS5Ob3JtYWxpemVkTGFuZG1hcmtMaXN0TGlzdCk6XG4gICAgTm9ybWFsaXplZExhbmRtYXJrTGlzdExpc3Qge1xuICBjb25zdCBvdXRwdXQgPSBbXTtcbiAgY29uc3QgcmVzdWx0Q291bnQgPSBsYW5kbWFya3Muc2l6ZSgpO1xuICBmb3IgKGxldCBpbmRleCA9IDA7IGluZGV4IDwgcmVzdWx0Q291bnQ7ICsraW5kZXgpIHtcbiAgICBjb25zdCBsYW5kbWFyayA9IGxhbmRtYXJrcy5nZXQoaW5kZXgpO1xuICAgIG91dHB1dC5wdXNoKGRlY29kZUxhbmRtYXJrcyhsYW5kbWFyaykpO1xuICB9XG4gIHJldHVybiBvdXRwdXQ7XG59XG5cbmZ1bmN0aW9uIGRlY29kZUNsYXNzaWZpY2F0aW9ucyhcbiAgICBjbGFzc2lmaWNhdGlvbnM6IHNvbHV0aW9uc1dhc20uQ2xhc3NpZmljYXRpb25MaXN0KTogQ2xhc3NpZmljYXRpb25MaXN0IHtcbiAgY29uc3Qgb3V0cHV0ID0gW107XG4gIGNvbnN0IHJlc3VsdENvdW50ID0gY2xhc3NpZmljYXRpb25zLnNpemUoKTtcbiAgZm9yIChsZXQgaW5kZXggPSAwOyBpbmRleCA8IHJlc3VsdENvdW50OyArK2luZGV4KSB7XG4gICAgY29uc3QgY2xhc3NpZmljYXRpb24gPSBjbGFzc2lmaWNhdGlvbnMuZ2V0KGluZGV4KTtcbiAgICBvdXRwdXQucHVzaCh7XG4gICAgICBpbmRleDogY2xhc3NpZmljYXRpb24uaW5kZXgsXG4gICAgICBzY29yZTogY2xhc3NpZmljYXRpb24uc2NvcmUsXG4gICAgICBsYWJlbDogY2xhc3NpZmljYXRpb24uaGFzTGFiZWwgPyBjbGFzc2lmaWNhdGlvbi5sYWJlbCA6IHVuZGVmaW5lZCxcbiAgICAgIGRpc3BsYXlOYW1lOiBjbGFzc2lmaWNhdGlvbi5oYXNEaXNwbGF5TmFtZSA/IGNsYXNzaWZpY2F0aW9uLmRpc3BsYXlOYW1lIDpcbiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIHVuZGVmaW5lZCxcbiAgICB9KTtcbiAgfVxuICByZXR1cm4gb3V0cHV0O1xufVxuXG5mdW5jdGlvbiBkZWNvZGVDbGFzc2lmaWNhdGlvbnNMaXN0KFxuICAgIGNsYXNzaWZpY2F0aW9uczogc29sdXRpb25zV2FzbS5DbGFzc2lmaWNhdGlvbkxpc3RMaXN0KTpcbiAgICBDbGFzc2lmaWNhdGlvbkxpc3RMaXN0IHtcbiAgY29uc3Qgb3V0cHV0ID0gW107XG4gIGNvbnN0IHJlc3VsdENvdW50ID0gY2xhc3NpZmljYXRpb25zLnNpemUoKTtcbiAgZm9yIChsZXQgaW5kZXggPSAwOyBpbmRleCA8IHJlc3VsdENvdW50OyArK2luZGV4KSB7XG4gICAgY29uc3QgY2xhc3NpZmljYXRpb24gPSBjbGFzc2lmaWNhdGlvbnMuZ2V0KGluZGV4KTtcbiAgICBvdXRwdXQucHVzaChkZWNvZGVDbGFzc2lmaWNhdGlvbnMoY2xhc3NpZmljYXRpb24pKTtcbiAgfVxuICByZXR1cm4gb3V0cHV0O1xufVxuXG5mdW5jdGlvbiBkZWNvZGVLZXlwb2ludChrZXlwb2ludDogc29sdXRpb25zV2FzbS5LZXlwb2ludCk6IEtleXBvaW50IHtcbiAgcmV0dXJuIHtcbiAgICBpZDoga2V5cG9pbnQuaWQsXG4gICAgcG9pbnQzZDoge1xuICAgICAgeDoga2V5cG9pbnQucG9pbnQzZC54LFxuICAgICAgeToga2V5cG9pbnQucG9pbnQzZC55LFxuICAgICAgejoga2V5cG9pbnQucG9pbnQzZC56LFxuICAgIH0sXG4gICAgcG9pbnQyZDoge1xuICAgICAgeDoga2V5cG9pbnQucG9pbnQyZC54LFxuICAgICAgeToga2V5cG9pbnQucG9pbnQyZC55LFxuICAgICAgZGVwdGg6IGtleXBvaW50LnBvaW50MmQuZGVwdGgsXG4gICAgfVxuICB9O1xufVxuXG5mdW5jdGlvbiBkZWNvZGVLZXlwb2ludExpc3QoXG4gICAga2V5cG9pbnRzOiBzb2x1dGlvbnNXYXNtLktleXBvaW50TGlzdCk6IEtleXBvaW50TGlzdCB7XG4gIGNvbnN0IG91dHB1dCA9IFtdO1xuICBjb25zdCByZXN1bHRDb3VudCA9IGtleXBvaW50cy5zaXplKCk7XG4gIGZvciAobGV0IGluZGV4ID0gMDsgaW5kZXggPCByZXN1bHRDb3VudDsgKytpbmRleCkge1xuICAgIGNvbnN0IGtleXBvaW50ID0ga2V5cG9pbnRzLmdldChpbmRleCk7XG4gICAgb3V0cHV0LnB1c2goZGVjb2RlS2V5cG9pbnQoa2V5cG9pbnQpKTtcbiAgfVxuICByZXR1cm4gb3V0cHV0O1xufVxuXG5mdW5jdGlvbiBkZWNvZGVPYmplY3REZXRlY3Rpb24oXG4gICAgZGV0ZWN0aW9uOiBzb2x1dGlvbnNXYXNtLk9iamVjdERldGVjdGlvbik6IE9iamVjdERldGVjdGlvbiB7XG4gIHJldHVybiB7XG4gICAgaWQ6IGRldGVjdGlvbi5pZCxcbiAgICBrZXlwb2ludHM6IGRlY29kZUtleXBvaW50TGlzdChkZXRlY3Rpb24ua2V5cG9pbnRzKSxcbiAgICB2aXNpYmlsaXR5OiBkZXRlY3Rpb24udmlzaWJpbGl0eSxcbiAgfTtcbn1cblxuZnVuY3Rpb24gZGVjb2RlT2JqZWN0RGV0ZWN0aW9uTGlzdChcbiAgICBkZXRlY3Rpb25zOiBzb2x1dGlvbnNXYXNtLk9iamVjdERldGVjdGlvbkxpc3QpOiBPYmplY3REZXRlY3Rpb25MaXN0IHtcbiAgY29uc3Qgb3V0cHV0ID0gW107XG4gIGNvbnN0IHJlc3VsdENvdW50ID0gZGV0ZWN0aW9ucy5zaXplKCk7XG4gIGZvciAobGV0IGluZGV4ID0gMDsgaW5kZXggPCByZXN1bHRDb3VudDsgKytpbmRleCkge1xuICAgIGNvbnN0IGRldGVjdGlvbiA9IGRldGVjdGlvbnMuZ2V0KGluZGV4KTtcbiAgICBvdXRwdXQucHVzaChkZWNvZGVPYmplY3REZXRlY3Rpb24oZGV0ZWN0aW9uKSk7XG4gIH1cbiAgcmV0dXJuIG91dHB1dDtcbn1cblxuY2xhc3MgR3JhcGhEYXRhSW1wbCB7XG4gIGNvbnN0cnVjdG9yKFxuICAgICAgcHJpdmF0ZSByZWFkb25seSBncmFwaDogR3JhcGgsIHByaXZhdGUgcmVhZG9ubHkgbG9jYXRlRmlsZTogRmlsZUxvY2F0b3JGbixcbiAgICAgIHByaXZhdGUgcmVhZG9ubHkgcGFja2FnZVBhdGg6IHN0cmluZykge31cblxuICBhc3luYyB0b0FycmF5QnVmZmVyKCk6IFByb21pc2U8QXJyYXlCdWZmZXI+IHtcbiAgICBpZiAodGhpcy5ncmFwaC51cmwpIHtcbiAgICAgIGNvbnN0IGZldGNoZWQgPVxuICAgICAgICAgIGF3YWl0IGZldGNoKHRoaXMubG9jYXRlRmlsZSh0aGlzLmdyYXBoLnVybCwgdGhpcy5wYWNrYWdlUGF0aCkpO1xuICAgICAgaWYgKGZldGNoZWQuYm9keSkge1xuICAgICAgICByZXR1cm4gZmV0Y2hlZC5hcnJheUJ1ZmZlcigpO1xuICAgICAgfVxuICAgIH1cbiAgICByZXR1cm4gbmV3IEFycmF5QnVmZmVyKDApO1xuICB9XG59XG5cbi8qKlxuICogSW5wdXRzIHRvIHRoZSBncmFwaC4gQ3VycmVudGx5IG9ubHkgb25lIGlucHV0LCBhIHZpZGVvIGZyYW1lLCBpc1xuICogcGVybWl0dGVkLCBidXQgdGhpcyBzaG91bGQgZW5jb21wYXNzIGFueSBpbnB1dCBkYXRhIHRvIGEgc29sdXRpb24uXG4gKi9cbmV4cG9ydCBpbnRlcmZhY2UgRnJhbWVJbnB1dHMge1xuICB2aWRlbzogSFRNTFZpZGVvRWxlbWVudDtcbn1cblxuLyoqXG4gKiBUaGUgc29sdXRpb24ga2VlcHMgYWxsIG9mIHRoZSBjdXJyZW50IGxpc3RlbmVycy4gVGhlIHVzZXIgY2FuIHBhc3MgdXMgYVxuICogbGlzdGVuZXIgbWFwIGFuZCB0aGVuIGNoYW5nZSB0aGUgZWxlbWVudHMgYXQgYW55IHRpbWUuIFRoZXkgd2lsbCBiZSBwaWNrZWRcbiAqIHVwIG9uIHRoZSBuZXh0IHBhc3MgdGhyb3VnaCBgcHJvY2Vzc0ZyYW1lYC5cbiAqL1xuZXhwb3J0IGludGVyZmFjZSBMaXN0ZW5lck1hcCB7XG4gIFtrZXk6IHN0cmluZ106IFJlc3VsdE1hcExpc3RlbmVyfHVuZGVmaW5lZDtcbn1cblxuYXN5bmMgZnVuY3Rpb24gaXNTdXBwb3J0ZWRTaW1kKCk6IFByb21pc2U8Ym9vbGVhbj4ge1xuICB0cnkge1xuICAgIGF3YWl0IFdlYkFzc2VtYmx5Lmluc3RhbnRpYXRlKFdBU01fU0lNRF9DSEVDSyk7XG4gIH0gY2F0Y2gge1xuICAgIHJldHVybiBmYWxzZTtcbiAgfVxuICByZXR1cm4gdHJ1ZTtcbn1cblxuLyoqXG4gKiBNZWRpYVBpcGUgU29sdXRpb24gdXBvbiB3aGljaCBhbGwgc3BlY2lmaWMgc29sdXRpb25zIHdpbGwgYmUgYnVpbHQuXG4gKi9cbmV4cG9ydCBjbGFzcyBTb2x1dGlvbiB7XG4gIHByaXZhdGUgcmVhZG9ubHkgcGFja2FnZVBhdGg6IHN0cmluZztcbiAgcHJpdmF0ZSByZWFkb25seSBsb2NhdGVGaWxlOiBGaWxlTG9jYXRvckZuO1xuICBwcml2YXRlIHJlYWRvbmx5IGxpc3RlbmVyczogTGlzdGVuZXJNYXAgPSB7fTtcbiAgcHJpdmF0ZSByZWFkb25seSBpblN0cmVhbUhlbHBlcnM6IHN0cmVhbUhlbHBlcnMuU3RyZWFtSGVscGVyTWFwID0ge307XG4gIHByaXZhdGUgcmVhZG9ubHkgb3V0U3RyZWFtSGVscGVyczogc3RyZWFtSGVscGVycy5TdHJlYW1IZWxwZXJNYXAgPSB7fTtcblxuICAvLyBCRUdJTjogQXNzaWduZWQgZHVyaW5nIGluaXRpYWxpemVXYXNtLi4uXG4gIHByaXZhdGUgZ2xDYW52YXMhOiBIVE1MQ2FudmFzRWxlbWVudDtcbiAgcHJpdmF0ZSBnbCE6IFdlYkdMMlJlbmRlcmluZ0NvbnRleHQ7XG4gIHByaXZhdGUgd2FzbSE6IHNvbHV0aW9uc1dhc20uTWVkaWFwaXBlV2FzbTtcbiAgcHJpdmF0ZSBzb2x1dGlvbldhc20hOiBzb2x1dGlvbnNXYXNtLlNvbHV0aW9uV2FzbTtcbiAgLy8gRU5EOiBBc3NpZ25lZCBkdXJpbmcgaW5pdGlhbGl6ZVdhc20uLi5cblxuICBwcml2YXRlIHBlbmRpbmdDaGFuZ2VzPzogc29sdXRpb25zV2FzbS5HcmFwaE9wdGlvbkNoYW5nZVJlcXVlc3RbXTtcblxuICBwcml2YXRlIG5lZWRUb0luaXRpYWxpemVXYXNtID0gdHJ1ZTtcbiAgcHJpdmF0ZSBuZWVkVG9Jbml0aWFsaXplR3JhcGggPSB0cnVlO1xuXG4gIHByaXZhdGUgcnVubmluZ1Byb21pc2UgPSBQcm9taXNlLnJlc29sdmUoKTtcblxuICBjb25zdHJ1Y3Rvcihwcml2YXRlIHJlYWRvbmx5IGNvbmZpZzogU29sdXRpb25Db25maWcpIHtcbiAgICB0aGlzLmxvY2F0ZUZpbGUgPSAoY29uZmlnICYmIGNvbmZpZy5sb2NhdGVGaWxlKSB8fCBkZWZhdWx0TG9jYXRlRmlsZTtcbiAgICB0aGlzLnBhY2thZ2VQYXRoID0gZ2V0UGFja2FnZVBhdGgoKTtcbiAgfVxuXG4gIGNsb3NlKCk6IFByb21pc2U8dm9pZD4ge1xuICAgIGlmICh0aGlzLnNvbHV0aW9uV2FzbSkge1xuICAgICAgdGhpcy5zb2x1dGlvbldhc20uZGVsZXRlKCk7XG4gICAgfVxuICAgIHJldHVybiBQcm9taXNlLnJlc29sdmUoKTtcbiAgfVxuXG4gIC8qKlxuICAgKiBMb2FkcyBhbGwgb2YgdGhlIGRlcGVuZGVudCBXQVNNIGZpbGVzLiBUaGlzIGlzIGhlYXZ5LCBzbyB3ZSBtYWtlIHN1cmUgdG9cbiAgICogb25seSBkbyB0aGlzIG9uY2UuXG4gICAqL1xuICBwcml2YXRlIGFzeW5jIHRyeVRvSW5pdGlhbGl6ZVdhc20oKTogUHJvbWlzZTx2b2lkPiB7XG4gICAgaWYgKCF0aGlzLm5lZWRUb0luaXRpYWxpemVXYXNtKSB7XG4gICAgICByZXR1cm47XG4gICAgfVxuICAgIC8vIFNldCB1cCB0aGUgZmlsZSBsb2FkZXIgZm9yIGJvdGggZXh0ZXJuYWwgbG9hZGVycy5cbiAgICBzZXRXaW5kb3coQVBJX0VYUE9SVF9OQU1FLCB7bG9jYXRlRmlsZTogdGhpcy5sb2NhdGVGaWxlfSk7XG4gICAgc2V0V2luZG93KFBBQ0tfRVhQT1JUX05BTUUsIHtsb2NhdGVGaWxlOiB0aGlzLmxvY2F0ZUZpbGV9KTtcblxuICAgIGNvbnN0IGZpbGVzID0gKHRoaXMuY29uZmlnLmZpbGVzKSB8fCBbXTtcbiAgICBjb25zdCBzdXBwb3J0c1NpbWQgPSBhd2FpdCBpc1N1cHBvcnRlZFNpbWQoKTtcblxuICAgIGF3YWl0IFByb21pc2UuYWxsKGZpbGVzLm1hcCh4ID0+IHtcbiAgICAgIGNvbnN0IGRvTG9hZCA9ICh4LnNpbWQgPT09IHVuZGVmaW5lZCkgfHwgKHguc2ltZCAmJiBzdXBwb3J0c1NpbWQpIHx8XG4gICAgICAgICAgKCF4LnNpbWQgJiYgIXN1cHBvcnRzU2ltZCk7XG4gICAgICBpZiAoZG9Mb2FkKSB7XG4gICAgICAgIHJldHVybiBsb2FkU2NyaXB0KHRoaXMubG9jYXRlRmlsZSh4LnVybCwgdGhpcy5wYWNrYWdlUGF0aCkpO1xuICAgICAgfVxuICAgICAgcmV0dXJuIFByb21pc2UucmVzb2x2ZSgpO1xuICAgIH0pKTtcblxuICAgIC8vIFRoZSB2YXJpYWJsZXMgd2Ugc2V0IGVhcmxpZXIgd2lsbCBub3QgYmUgbXV0YXRlZCwgZWFjaCBhY2NvcmRpbmcgdG8gaXRzXG4gICAgLy8gcmVsYXRlZCBsb2FkZXIuXG4gICAgY29uc3QgYXBpRm4gPSBnZXRXaW5kb3coQVBJX0VYUE9SVF9OQU1FKSBhcyBzb2x1dGlvbnNXYXNtLkFwaVByb21pc2VGbjtcbiAgICBjb25zdCBwYWNrRm4gPSBnZXRXaW5kb3coUEFDS19FWFBPUlRfTkFNRSkgYXMgc29sdXRpb25zV2FzbS5QYWNrTG9hZGVyO1xuXG4gICAgLy8gTm93IHRoYXQgZXZlcnl0aGluZyBpcyBsb2FkZWQgYW5kIG11dGF0ZWQsIHdlIGNhbiBmaW5hbGx5IGluaXRpYWxpemVcbiAgICAvLyB0aGUgV0FTTSBsb2FkZXIgd2l0aCB0aGUgcGFjayBsb2FkZXIuIFRoZSBXQVNNIGxvYWRlciB3aWxsIHdhaXQgdW50aWxcbiAgICAvLyBhbGwgb2YgdGhlIGZpbGVzIGluIHRoZSBwYWNrIGxvYWRlciBhcmUgY29tcGxldGUgYmVmb3JlIHJlc29sdmluZyBpdHNcbiAgICAvLyBQcm9taXNlLlxuICAgIHRoaXMud2FzbSA9IGF3YWl0IGFwaUZuKHBhY2tGbik7XG5cbiAgICAvLyBUT0RPKG1oYXlzKTogRGV2ZWxvcGVyIHNob3VsZCBiZSBhYmxlIHRvIGV4cGxpY2l0bHkgbG9hZC91bmxvYWQgYVxuICAgIC8vIHNvbHV0aW9uIHRvIHByZXZlbnQgc3RlYWxpbmcgYWxsIG9mIHRoZSBXZWJHTCByZXNvdXJjZXMgKGUuZy4sIENocm9tZVxuICAgIC8vIG1heSBsaW1pdCB0aGUgbnVtYmVyIG9mIFdlYkdMIGNvbnRleHRzIGJ5IGRvbWFpbikuXG5cbiAgICAvLyBUaGlzIGlzIHRoZSBzaW5nbGUgY2FudmFzIHRoYXQgd2lsbCBiZSB1c2VkIGZvciBhbGwgdGV4dHVyZSB0cmFuc2ZlclxuICAgIC8vIHRvIGFuZCBmcm9tIHRoZSBiYWNrIGVuZC5cbiAgICB0aGlzLmdsQ2FudmFzID0gZG9jdW1lbnQuY3JlYXRlRWxlbWVudCgnY2FudmFzJyk7XG4gICAgdGhpcy53YXNtLmNhbnZhcyA9IHRoaXMuZ2xDYW52YXM7XG5cbiAgICB0aGlzLndhc20uY3JlYXRlQ29udGV4dChcbiAgICAgICAgdGhpcy5nbENhbnZhcywgLyogdXNlV2ViR2w9ICovIHRydWUsXG4gICAgICAgIC8qIHNldEluTW9kdWxlPSAqLyB0cnVlLCB7fSk7XG5cbiAgICAvLyBUaGUgZ3JhcGggb25seSBuZWVkcyB0byBiZSBsb2FkZWQgb25jZSBpbnRvIHRoZSBzb2x1dGlvbiwgYnV0IHRoZSBXQVNNXG4gICAgLy8gbWlnaHQgcmUtaW5pdGlhbGl6ZSB0aGUgc29sdXRpb24sIGFuZCB0aGF0IHdpbGwgc3BlY2lmaWNhbGx5IGhhcHBlblxuICAgIC8vIGR1cmluZyB3YXNtLlByb2Nlc3NGcmFtZS5cbiAgICB0aGlzLnNvbHV0aW9uV2FzbSA9IG5ldyB0aGlzLndhc20uU29sdXRpb25XYXNtKCk7XG4gICAgY29uc3QgZ3JhcGhEYXRhID1cbiAgICAgICAgbmV3IEdyYXBoRGF0YUltcGwodGhpcy5jb25maWcuZ3JhcGgsIHRoaXMubG9jYXRlRmlsZSwgdGhpcy5wYWNrYWdlUGF0aCk7XG4gICAgYXdhaXQgdGhpcy5sb2FkR3JhcGgoZ3JhcGhEYXRhKTtcblxuICAgIC8vIExpc3RlbmVycyBtb2RpZnkgdGhlIGdyYXBoIGNvbmZpZyBpbnRlcm5hbGx5LCBzbyBncmFwaCBsaXN0ZW5lcnMgYXJlXG4gICAgLy8gYXR0YWNoZWQgb25seSBvbmNlLiBVc2VycyBhcmUgZnJlZSB0byBjb25uZWN0IGFuZCBkaXNjb25uZWN0IGNhbGxiYWNrc1xuICAgIC8vIHRvIHRoZXNlIGxpc3RlbmVycyBhcyBuZWVkZWQuXG4gICAgaWYgKHRoaXMuY29uZmlnLmxpc3RlbmVycykge1xuICAgICAgZm9yIChjb25zdCBsaXN0ZW5lciBvZiB0aGlzLmNvbmZpZy5saXN0ZW5lcnMpIHtcbiAgICAgICAgdGhpcy5hZGRMaXN0ZW5lclBvcnQobGlzdGVuZXIpO1xuICAgICAgfVxuICAgIH1cblxuICAgIHRoaXMubmVlZFRvSW5pdGlhbGl6ZVdhc20gPSBmYWxzZTtcbiAgfVxuXG4gIC8qKlxuICAgKiBTZXRzIHRoZSBvcHRpb25zIGZvciB0aGUgZ3JhcGgsIHBvdGVudGlhbGx5IHRyaWdnZXJpbmcgYSByZWluaXRpYWxpemUuXG4gICAqIFRoZSB0cmlnZ2VyaW5nIGlzIGNvbnRpbmdlbnQgdXBvbiB0aGUgb3B0aW9ucyBtYXRjaGluZyB0aG9zZSBzZXQgdXAgaW5cbiAgICogdGhlIHNvbHV0aW9uIGNvbmZpZ3VyYXRpb24uIElmIGEgbWF0Y2ggaXMgZm91bmQsIGluaXRpYWxpemUgaXMgc2V0IHRvIHJ1blxuICAgKiBvbiB0aGUgbmV4dCBwcm9jZXNzRnJhbWUuXG4gICAqXG4gICAqIFdlIGRvIG5vdCBjcmVhdGUgYSBXQVNNIG9iamVjdCBoZXJlLCBiZWNhdXNlIGl0J3MgcG9zc2libGUgKGxpa2VseSkgdGhhdFxuICAgKiBXQVNNIGhhcyBub3QgbG9hZGVkIHlldCAoaS5lLiwgdGhlIHVzZXIgY2FsbHMgc2V0T3B0aW9ucyBiZWZvcmUgY2FsbGluZ1xuICAgKiBwcm9jZXNzRnJhbWUgLyBpbml0aWFsaXplKS4gIFdlJ2xsIGRvIHRoYXQgZHVyaW5nIGluaXRpYWxpemUgd2hlbiB3ZSBrbm93XG4gICAqIGl0J3Mgc2FmZS5cbiAgICovXG4gIHNldE9wdGlvbnMob3B0aW9uczogT3B0aW9uTGlzdCk6IHZvaWQge1xuICAgIGlmICghdGhpcy5jb25maWcub3B0aW9ucykge1xuICAgICAgcmV0dXJuO1xuICAgIH1cbiAgICBjb25zdCBwZW5kaW5nQ2hhbmdlczogc29sdXRpb25zV2FzbS5HcmFwaE9wdGlvbkNoYW5nZVJlcXVlc3RbXSA9IFtdO1xuICAgIGZvciAoY29uc3Qgb3B0aW9uIG9mIE9iamVjdC5rZXlzKG9wdGlvbnMpKSB7XG4gICAgICAvLyBMb29rIGVhY2ggb3B0aW9uIGluIHRoZSBvcHRpb24gY29uZmlnLlxuICAgICAgY29uc3Qgb3B0aW9uQ29uZmlnID0gdGhpcy5jb25maWcub3B0aW9uc1tvcHRpb25dO1xuICAgICAgaWYgKG9wdGlvbkNvbmZpZykge1xuICAgICAgICBpZiAob3B0aW9uQ29uZmlnLmdyYXBoT3B0aW9uWHJlZikge1xuICAgICAgICAgIGNvbnN0IG9wdGlvblZhbHVlID0ge1xuICAgICAgICAgICAgdmFsdWVOdW1iZXI6IG9wdGlvbkNvbmZpZy50eXBlID09PSBPcHRpb25UeXBlLk5VTUJFUiA/XG4gICAgICAgICAgICAgICAgb3B0aW9uc1tvcHRpb25dIGFzIG51bWJlciA6XG4gICAgICAgICAgICAgICAgMC4wLFxuICAgICAgICAgICAgdmFsdWVCb29sZWFuOiBvcHRpb25Db25maWcudHlwZSA9PT0gT3B0aW9uVHlwZS5CT09MID9cbiAgICAgICAgICAgICAgICBvcHRpb25zW29wdGlvbl0gYXMgYm9vbGVhbiA6XG4gICAgICAgICAgICAgICAgZmFsc2VcbiAgICAgICAgICB9O1xuICAgICAgICAgIGNvbnN0IGRlZmF1bHRzID0ge2NhbGN1bGF0b3JOYW1lOiAnJywgY2FsY3VsYXRvckluZGV4OiAwfTtcbiAgICAgICAgICAvLyBDb21iaW5lIHRoZSB4cmVmIHdpdGggdGhlIHZhbHVlLiBUaGlzIGlzIHdoYXQgdGhlIFdBU00gd2lsbCBiZVxuICAgICAgICAgIC8vIGV4cGVjdGluZy5cbiAgICAgICAgICBjb25zdCByZXF1ZXN0ID0ge1xuICAgICAgICAgICAgLi4uZGVmYXVsdHMsXG4gICAgICAgICAgICAuLi5vcHRpb25Db25maWcuZ3JhcGhPcHRpb25YcmVmLFxuICAgICAgICAgICAgLi4ub3B0aW9uVmFsdWVcbiAgICAgICAgICB9O1xuICAgICAgICAgIHBlbmRpbmdDaGFuZ2VzLnB1c2gocmVxdWVzdCk7XG4gICAgICAgIH1cbiAgICAgIH1cbiAgICB9XG5cbiAgICBpZiAocGVuZGluZ0NoYW5nZXMubGVuZ3RoICE9PSAwKSB7XG4gICAgICB0aGlzLm5lZWRUb0luaXRpYWxpemVHcmFwaCA9IHRydWU7XG4gICAgICB0aGlzLnBlbmRpbmdDaGFuZ2VzID0gcGVuZGluZ0NoYW5nZXM7XG4gICAgfVxuICB9XG5cbiAgLyoqXG4gICAqIEluaXRpYWxpemVzIHRoZSBncmFwaCBpcyBpdCBoYXMgbm90IGJlZW4gbG9hZGVkLCBvciBoYXMgYmVlbiB0cmlnZ2VyZWQgdG9cbiAgICogcmVsb2FkIChzZXRPcHRpb25zIHdhcyBjYWxsZWQgd2hpbGUgdGhlIGdyYXBoIHdhcyBydW5uaW5nKS5cbiAgICovXG4gIHByaXZhdGUgYXN5bmMgdHJ5VG9Jbml0aWFsaXplR3JhcGgoKSB7XG4gICAgaWYgKCF0aGlzLm5lZWRUb0luaXRpYWxpemVHcmFwaCkge1xuICAgICAgcmV0dXJuO1xuICAgIH1cbiAgICAvLyBNb3ZlIHRoZSB2aWRlbyBmcmFtZSBpbnRvIHRoZSB0ZXh0dXJlLlxuICAgIGNvbnN0IGdsID0gdGhpcy5nbENhbnZhcy5nZXRDb250ZXh0KCd3ZWJnbDInKTtcbiAgICBpZiAoIWdsKSB7XG4gICAgICBhbGVydCgnRmFpbGVkIHRvIGNyZWF0ZSBXZWJHTCBjYW52YXMgY29udGV4dCB3aGVuIHBhc3NpbmcgdmlkZW8gZnJhbWUuJyk7XG4gICAgICByZXR1cm47XG4gICAgfVxuICAgIHRoaXMuZ2wgPSBnbDtcblxuICAgIC8vIENoYW5naW5nIG9wdGlvbnMgb24gdGhlIGdyYXBoIHdpbGwgbXV0YXRlIHRoZSBncmFwaCBjb25maWcuXG4gICAgaWYgKHRoaXMucGVuZGluZ0NoYW5nZXMpIHtcbiAgICAgIGNvbnN0IGNoYW5nZUxpc3QgPSBuZXcgdGhpcy53YXNtLkdyYXBoT3B0aW9uQ2hhbmdlUmVxdWVzdExpc3QoKTtcbiAgICAgIGZvciAoY29uc3QgY2hhbmdlIG9mIHRoaXMucGVuZGluZ0NoYW5nZXMpIHtcbiAgICAgICAgY2hhbmdlTGlzdC5wdXNoX2JhY2soY2hhbmdlKTtcbiAgICAgIH1cbiAgICAgIHRoaXMuc29sdXRpb25XYXNtLmNoYW5nZU9wdGlvbnMoY2hhbmdlTGlzdCk7XG4gICAgICBjaGFuZ2VMaXN0LmRlbGV0ZSgpO1xuICAgICAgdGhpcy5wZW5kaW5nQ2hhbmdlcyA9IHVuZGVmaW5lZDtcbiAgICB9XG5cbiAgICB0aGlzLm5lZWRUb0luaXRpYWxpemVHcmFwaCA9IGZhbHNlO1xuICB9XG5cbiAgYXN5bmMgaW5pdGlhbGl6ZSgpOiBQcm9taXNlPHZvaWQ+IHtcbiAgICBhd2FpdCB0aGlzLnRyeVRvSW5pdGlhbGl6ZVdhc20oKTtcbiAgICBhd2FpdCB0aGlzLnRyeVRvSW5pdGlhbGl6ZUdyYXBoKCk7XG4gIH1cblxuICBwcml2YXRlIGFzeW5jIGxvYWRHcmFwaChncmFwaDogR3JhcGhEYXRhKTogUHJvbWlzZTx2b2lkPiB7XG4gICAgY29uc3QgZ3JhcGhEYXRhID0gYXdhaXQgZ3JhcGgudG9BcnJheUJ1ZmZlcigpO1xuICAgIHRoaXMuc29sdXRpb25XYXNtLmxvYWRHcmFwaChncmFwaERhdGEpO1xuICB9XG5cbiAgLyoqXG4gICAqIFVzZXMgdGhlICd0eXBlJyBmaWVsZCBvZiB0aGUgaW5wdXQgY29uZmlnIHRvIHR1cm4gdGhlIGB1bmtub3duYCBpbnB1dCBpbnRvXG4gICAqIHNvbWV0aGluZyB1c2FibGUgYnkgdGhlIGJhY2sgZW5kLiAgSW4gZ2VuZXJhbCwgd2UgYXJlIHVzaW5nIGVtc2NyaXB0ZW5cbiAgICogZW1iaW5kJ3MgdmFsdWVfb2JqZWN0IHRvIGVmZmVjdCB0aGUgdHJhbnNmZXIsIHNvIHRoZSBkYXRhIG11c3QgYmUgc2ltcGx5XG4gICAqIG1hcnNoYWxhYmxlLlxuICAgKi9cbiAgcHJpdmF0ZSBwcmVwYXJlSW5wdXQoY29uZmlnOiBJbnB1dENvbmZpZywgaW5wdXQ6IHVua25vd24pOiBvYmplY3Qge1xuICAgIHN3aXRjaCAoY29uZmlnLnR5cGUpIHtcbiAgICAgIGNhc2UgJ3ZpZGVvJzpcbiAgICAgICAgbGV0IGhlbHBlciA9IHRoaXMuaW5TdHJlYW1IZWxwZXJzW2NvbmZpZy5zdHJlYW1dIGFzXG4gICAgICAgICAgICBzdHJlYW1IZWxwZXJzLkltYWdlU3RyZWFtSGVscGVyO1xuICAgICAgICBpZiAoIWhlbHBlcikge1xuICAgICAgICAgIGhlbHBlciA9IG5ldyBzdHJlYW1IZWxwZXJzLkltYWdlU3RyZWFtSGVscGVyKHRoaXMud2FzbSwgdGhpcy5nbCk7XG4gICAgICAgICAgdGhpcy5pblN0cmVhbUhlbHBlcnNbY29uZmlnLnN0cmVhbV0gPSBoZWxwZXI7XG4gICAgICAgIH1cbiAgICAgICAgcmV0dXJuIGhlbHBlci5lbmNvZGUoaW5wdXQgYXMgSW5wdXRJbWFnZSk7XG4gICAgICBkZWZhdWx0OlxuICAgICAgICByZXR1cm4ge307XG4gICAgfVxuICB9XG4gIC8qKlxuICAgKiBBbGxvd3MgdGhlIHVzZXIgdG8gc2VuZCBvbmUgb3IgbW9yZSBpbnB1dHMgdG8gdGhlIGdyYXBoLiBUaGUgZ3JhcGggd2lsbFxuICAgKiBydW4gYW5kIHBvdGVudGlhbGx5IHByb2R1Y2Ugb3V0cHV0cyBmb3IgYW55IGF0dGFjaGVkIGxpc3RlbmVycy4gVGhlIGRhdGFcbiAgICogdGhhdCBhIGphdmFzY3JpcHQgdXNlciBzdXBwbGllcyBuZWVkcyB0byBiZSBjb25kaXRpb25lZCBmb3IgdXNlIGJ5XG4gICAqIG1lZGlhcGlwZSwgYW5kIHRob3NlIGRldGFpbHMgYXJlIGhpZGRlbiBhd2F5IChzZWUgYHByZXBhcmVJbnB1dGApLlxuICAgKiBAcGFyYW0gYXQgVGltZXN0YW1wIGluIG1zLCBpZiB1bmRlZmluZWQsIHdlJ2xsIHVzZSBwZXJmb3JtYW5jZS5ub3coKS5cbiAgICovXG4gIGFzeW5jIHNlbmQoaW5wdXRzOiBJbnB1dE1hcCwgYXQ/OiBudW1iZXIpOiBQcm9taXNlPHZvaWQ+IHtcbiAgICBpZiAoIXRoaXMuY29uZmlnLmlucHV0cykge1xuICAgICAgcmV0dXJuO1xuICAgIH1cbiAgICBjb25zdCB0aW1lc3RhbXAgPSAxMDAwICogKGF0IHx8IHBlcmZvcm1hbmNlLm5vdygpKTtcblxuICAgIGF3YWl0IHRoaXMucnVubmluZ1Byb21pc2U7XG4gICAgYXdhaXQgdGhpcy5pbml0aWFsaXplKCk7XG4gICAgY29uc3QgZGF0YVZhbHVlTGlzdCA9IG5ldyB0aGlzLndhc20uUGFja2V0RGF0YUxpc3QoKTtcbiAgICBmb3IgKGNvbnN0IGtleSBvZiBPYmplY3Qua2V5cyhpbnB1dHMpKSB7XG4gICAgICBjb25zdCBpbnB1dENvbmZpZyA9IHRoaXMuY29uZmlnLmlucHV0c1trZXldO1xuICAgICAgaWYgKGlucHV0Q29uZmlnKSB7XG4gICAgICAgIC8vIFdlIG9ubHkgc3VwcG9ydCB0ZXh0dXJlMmQgYXQgdGhlIG1vbWVudCwgYnV0IG9uY2Ugd2Ugc3VwcG9ydCBtb3JlLFxuICAgICAgICAvLyB0aGlzIHdpbGwgYmUgYSBzd2l0Y2ggc3RhdGVtZW50IHNvIHRoYXQgd2UgY2FsbCB0aGUgY29ycmVjdCBwdXNoXG4gICAgICAgIC8vIG1ldGhvZC5cbiAgICAgICAgY29uc3QgaW5wdXQgPSB0aGlzLnByZXBhcmVJbnB1dChpbnB1dENvbmZpZywgaW5wdXRzW2tleV0pO1xuICAgICAgICBjb25zdCBzdHJlYW0gPSBpbnB1dENvbmZpZy5zdHJlYW07XG4gICAgICAgIGRhdGFWYWx1ZUxpc3QucHVzaFRleHR1cmUyZChcbiAgICAgICAgICAgIHsuLi5pbnB1dCwgc3RyZWFtLCB0aW1lc3RhbXB9IGFzIHNvbHV0aW9uc1dhc20uVGV4dHVyZTJkRGF0YSk7XG4gICAgICB9XG4gICAgfVxuICAgIHRoaXMuc29sdXRpb25XYXNtLnNlbmQoZGF0YVZhbHVlTGlzdCk7XG4gICAgZGF0YVZhbHVlTGlzdC5kZWxldGUoKTtcbiAgfVxuXG4gIHByaXZhdGUgdHJhbnNmb3JtT3V0cHV0cyhyZXN1bHRzOiBXYXNtUmVzdWx0TWFwLCBjb25maWdNYXA/OiBPdXRwdXRDb25maWdNYXApOlxuICAgICAgUmVzdWx0TWFwIHtcbiAgICBpZiAoIWNvbmZpZ01hcCkge1xuICAgICAgcmV0dXJuIHJlc3VsdHM7XG4gICAgfVxuICAgIGNvbnN0IG91dHB1dHM6IFJlc3VsdE1hcCA9IHt9O1xuICAgIGZvciAoY29uc3Qga2V5IG9mIE9iamVjdC5rZXlzKGNvbmZpZ01hcCkpIHtcbiAgICAgIGNvbnN0IGNvbmZpZyA9IGNvbmZpZ01hcFtrZXldO1xuICAgICAgaWYgKHR5cGVvZiBjb25maWcgPT09ICdzdHJpbmcnKSB7XG4gICAgICAgIG91dHB1dHNba2V5XSA9IHRoaXMuZXh0cmFjdFdhc21SZXN1bHQoa2V5LCByZXN1bHRzW2NvbmZpZ10pO1xuICAgICAgfSBlbHNlIHtcbiAgICAgICAgY29uc3QgcmVzdWx0ID0gcmVzdWx0c1tjb25maWcuc3RyZWFtXTtcbiAgICAgICAgaWYgKHJlc3VsdCA9PT0gdW5kZWZpbmVkKSB7XG4gICAgICAgICAgY29udGludWU7XG4gICAgICAgIH1cbiAgICAgICAgaWYgKGNvbmZpZy50eXBlID09PSAnZGV0ZWN0aW9uX2xpc3QnKSB7XG4gICAgICAgICAgb3V0cHV0c1trZXldID0gZGVjb2RlRGV0ZWN0aW9uTGlzdChyZXN1bHQpO1xuICAgICAgICB9IGVsc2UgaWYgKGNvbmZpZy50eXBlID09PSAnbGFuZG1hcmtzJykge1xuICAgICAgICAgIGNvbnN0IHZhbHVlID0gcmVzdWx0LmdldExhbmRtYXJrcygpO1xuICAgICAgICAgIG91dHB1dHNba2V5XSA9IHZhbHVlID8gZGVjb2RlTGFuZG1hcmtzKHZhbHVlKSA6IHVuZGVmaW5lZDtcbiAgICAgICAgfSBlbHNlIGlmIChjb25maWcudHlwZSA9PT0gJ2xhbmRtYXJrc19saXN0Jykge1xuICAgICAgICAgIGNvbnN0IHZhbHVlID0gcmVzdWx0LmdldExhbmRtYXJrc0xpc3QoKTtcbiAgICAgICAgICBvdXRwdXRzW2tleV0gPSB2YWx1ZSA/IGRlY29kZUxhbmRtYXJrc0xpc3QodmFsdWUpIDogdW5kZWZpbmVkO1xuICAgICAgICB9IGVsc2UgaWYgKGNvbmZpZy50eXBlID09PSAncmVjdF9saXN0Jykge1xuICAgICAgICAgIGNvbnN0IHZhbHVlID0gcmVzdWx0LmdldFJlY3RMaXN0KCk7XG4gICAgICAgICAgb3V0cHV0c1trZXldID0gdmFsdWUgPyBkZWNvZGVSZWN0TGlzdCh2YWx1ZSkgOiB1bmRlZmluZWQ7XG4gICAgICAgIH0gZWxzZSBpZiAoY29uZmlnLnR5cGUgPT09ICdjbGFzc2lmaWNhdGlvbnNfbGlzdCcpIHtcbiAgICAgICAgICBjb25zdCB2YWx1ZSA9IHJlc3VsdC5nZXRDbGFzc2lmaWNhdGlvbnNMaXN0KCk7XG4gICAgICAgICAgb3V0cHV0c1trZXldID0gdmFsdWUgPyBkZWNvZGVDbGFzc2lmaWNhdGlvbnNMaXN0KHZhbHVlKSA6IHVuZGVmaW5lZDtcbiAgICAgICAgfSBlbHNlIGlmIChjb25maWcudHlwZSA9PT0gJ29iamVjdF9kZXRlY3Rpb25fbGlzdCcpIHtcbiAgICAgICAgICBjb25zdCB2YWx1ZSA9IHJlc3VsdC5nZXRPYmplY3REZXRlY3Rpb25MaXN0KCk7XG4gICAgICAgICAgb3V0cHV0c1trZXldID0gdmFsdWUgPyBkZWNvZGVPYmplY3REZXRlY3Rpb25MaXN0KHZhbHVlKSA6IHVuZGVmaW5lZDtcbiAgICAgICAgfSBlbHNlIGlmIChjb25maWcudHlwZSA9PT0gJ3RleHR1cmUnKSB7XG4gICAgICAgICAgbGV0IGhlbHBlciA9XG4gICAgICAgICAgICAgIHRoaXMub3V0U3RyZWFtSGVscGVyc1trZXldIGFzIHN0cmVhbUhlbHBlcnMuSW1hZ2VTdHJlYW1IZWxwZXI7XG4gICAgICAgICAgaWYgKCFoZWxwZXIpIHtcbiAgICAgICAgICAgIGhlbHBlciA9IG5ldyBzdHJlYW1IZWxwZXJzLkltYWdlU3RyZWFtSGVscGVyKHRoaXMud2FzbSwgdGhpcy5nbCk7XG4gICAgICAgICAgICB0aGlzLm91dFN0cmVhbUhlbHBlcnNba2V5XSA9IGhlbHBlcjtcbiAgICAgICAgICB9XG4gICAgICAgICAgb3V0cHV0c1trZXldID0gaGVscGVyLmRlY29kZShyZXN1bHQuZ2V0VGV4dHVyZTJkKCkhKTtcbiAgICAgICAgfSBlbHNlIHtcbiAgICAgICAgICB0aHJvdyBuZXcgRXJyb3IoYFVua25vd24gb3V0cHV0IGNvbmZpZyB0eXBlOiAnJHtjb25maWcudHlwZX0nYCk7XG4gICAgICAgIH1cbiAgICAgICAgaWYgKGNvbmZpZy50cmFuc2Zvcm0gJiYgb3V0cHV0c1trZXldKSB7XG4gICAgICAgICAgb3V0cHV0c1trZXldID0gY29uZmlnLnRyYW5zZm9ybShvdXRwdXRzW2tleV0pO1xuICAgICAgICB9XG4gICAgICB9XG4gICAgfVxuICAgIHJldHVybiBvdXRwdXRzO1xuICB9XG5cbiAgLyoqXG4gICAqIEV4dHJhY3RzIHRoZSByZXN1bHQgZnJvbSBhIGNvbGxlY3Rpb24gb2YgcmVzdWx0cy4gVGhpcyB0cmllcyB0byBhdXRvZGV0ZWN0XG4gICAqIHRoZSB0eXBlIGFuZCBkb2VzIG5vIHRyYW5zZm9ybWF0aW9uLiBBcnJheXMgd2lsbCBoYXZlIHRoZSBzbGlnaHRseSBtb3JlXG4gICAqIGF3a3dhcmQgZW1zY3JpcHRlbiBhY2Nlc3NvcnMuXG4gICAqIEBUT0RPKG1oYXlzKSBEZXByZWNhdGUgdGhpcyBtZXRob2QgYW5kIGFsd2F5cyBzcGVjaWZ5IHRoZSB0eXBlLlxuICAgKi9cbiAgcHJpdmF0ZSBleHRyYWN0V2FzbVJlc3VsdChzdHJlYW06IHN0cmluZywgcmVzdWx0OiBzb2x1dGlvbnNXYXNtLlJlc3VsdFdhc20pOlxuICAgICAgdW5rbm93biB7XG4gICAgaWYgKHJlc3VsdC5pc051bWJlcigpKSB7XG4gICAgICByZXR1cm4gcmVzdWx0LmdldE51bWJlcigpO1xuICAgIH1cbiAgICBpZiAocmVzdWx0LmlzUmVjdCgpKSB7XG4gICAgICByZXR1cm4gcmVzdWx0LmdldFJlY3QoKTtcbiAgICB9XG4gICAgaWYgKHJlc3VsdC5pc0xhbmRtYXJrcygpKSB7XG4gICAgICByZXR1cm4gcmVzdWx0LmdldExhbmRtYXJrcygpO1xuICAgIH1cbiAgICBpZiAocmVzdWx0LmlzTGFuZG1hcmtzTGlzdCgpKSB7XG4gICAgICByZXR1cm4gcmVzdWx0LmdldExhbmRtYXJrc0xpc3QoKTtcbiAgICB9XG4gICAgaWYgKHJlc3VsdC5pc0NsYXNzaWZpY2F0aW9uc0xpc3QoKSkge1xuICAgICAgcmV0dXJuIHJlc3VsdC5nZXRDbGFzc2lmaWNhdGlvbnNMaXN0KCk7XG4gICAgfVxuICAgIGlmIChyZXN1bHQuaXNPYmplY3REZXRlY3Rpb25MaXN0KCkpIHtcbiAgICAgIHJldHVybiByZXN1bHQuZ2V0T2JqZWN0RGV0ZWN0aW9uTGlzdCgpO1xuICAgIH1cbiAgICBpZiAocmVzdWx0LmlzVGV4dHVyZTJkKCkpIHtcbiAgICAgIGxldCBoZWxwZXIgPVxuICAgICAgICAgIHRoaXMub3V0U3RyZWFtSGVscGVyc1tzdHJlYW1dIGFzIHN0cmVhbUhlbHBlcnMuSW1hZ2VTdHJlYW1IZWxwZXI7XG4gICAgICBpZiAoIWhlbHBlcikge1xuICAgICAgICBoZWxwZXIgPSBuZXcgc3RyZWFtSGVscGVycy5JbWFnZVN0cmVhbUhlbHBlcih0aGlzLndhc20sIHRoaXMuZ2wpO1xuICAgICAgICB0aGlzLm91dFN0cmVhbUhlbHBlcnNbc3RyZWFtXSA9IGhlbHBlcjtcbiAgICAgIH1cbiAgICAgIHJldHVybiBoZWxwZXIuZGVjb2RlKHJlc3VsdC5nZXRUZXh0dXJlMmQoKSEpO1xuICAgIH1cbiAgICByZXR1cm4gdW5kZWZpbmVkO1xuICB9XG5cbiAgLyoqXG4gICAqIEF0dGFjaGVzIGEgbGlzdGVuZXIgdGhhdCB3aWxsIGJlIGNhbGxlZCB3aGVuIHRoZSBncmFwaCBwcm9kdWNlc1xuICAgKiBjb21wYXRpYmxlIHBhY2tldHMgb24gdGhlIG5hbWVkIHN0cmVhbS5cbiAgICovXG4gIHByaXZhdGUgYWRkTGlzdGVuZXJQb3J0KHBvcnQ6IExpc3RlbmVyUG9ydCk6IHZvaWQge1xuICAgIGNvbnN0IHBvcnROYW1lID0gcG9ydC5uYW1lIHx8IERFRkFVTFRfUE9SVF9OQU1FO1xuICAgIGNvbnN0IHdhbnRzQ29weSA9IFsuLi5wb3J0LndhbnRzXTtcbiAgICBjb25zdCB3YW50c1ZlY3RvciA9IG5ldyB0aGlzLndhc20uU3RyaW5nTGlzdCgpO1xuICAgIGZvciAoY29uc3Qgd2FudCBvZiBwb3J0LndhbnRzKSB7XG4gICAgICB3YW50c1ZlY3Rvci5wdXNoX2JhY2sod2FudCk7XG4gICAgfVxuICAgIGNvbnN0IHdhc21MaXN0ZW5lciA9IHtcbiAgICAgIG9uUmVzdWx0czogYXN5bmModmFsdWVzOiBzb2x1dGlvbnNXYXNtLlJlc3VsdFdhc21MaXN0KTogUHJvbWlzZTx2b2lkPiA9PiB7XG4gICAgICAgIGNvbnN0IHdhc21SZXN1bHRzOiBXYXNtUmVzdWx0TWFwID0ge307XG4gICAgICAgIGZvciAobGV0IGluZGV4ID0gMDsgaW5kZXggPCBwb3J0LndhbnRzLmxlbmd0aDsgKytpbmRleCkge1xuICAgICAgICAgIHdhc21SZXN1bHRzW3dhbnRzQ29weVtpbmRleF1dID0gdmFsdWVzLmdldChpbmRleCk7XG4gICAgICAgIH1cbiAgICAgICAgY29uc3QgcmVzdWx0cyA9IHRoaXMudHJhbnNmb3JtT3V0cHV0cyh3YXNtUmVzdWx0cywgcG9ydC5vdXRzKTtcbiAgICAgICAgY29uc3QgbGlzdGVuZXIgPSB0aGlzLmxpc3RlbmVyc1twb3J0TmFtZV07XG4gICAgICAgIGlmIChsaXN0ZW5lcikge1xuICAgICAgICAgIGF3YWl0IHRoaXMucnVubmluZ1Byb21pc2U7XG4gICAgICAgICAgY29uc3QgcmVzdWx0ID0gbGlzdGVuZXIocmVzdWx0cyk7XG4gICAgICAgICAgaWYgKHJlc3VsdCkge1xuICAgICAgICAgICAgdGhpcy5ydW5uaW5nUHJvbWlzZSA9IHJlc3VsdDtcbiAgICAgICAgICAgIHJldHVybiByZXN1bHQ7XG4gICAgICAgICAgfVxuICAgICAgICB9XG4gICAgICB9XG4gICAgfTtcbiAgICBjb25zdCBwYWNrZXRMaXN0ZW5lciA9IHRoaXMud2FzbS5QYWNrZXRMaXN0ZW5lci5pbXBsZW1lbnQod2FzbUxpc3RlbmVyKTtcbiAgICB0aGlzLnNvbHV0aW9uV2FzbS5hdHRhY2hNdWx0aUxpc3RlbmVyKHdhbnRzVmVjdG9yLCBwYWNrZXRMaXN0ZW5lcik7XG4gICAgd2FudHNWZWN0b3IuZGVsZXRlKCk7XG4gIH1cblxuICAvKipcbiAgICogVXNlIHRoaXMgdG8gY29ubmVjdCB1cCB0byBvbmUgbGlzdGVuZXIgcGVyIHBvcnQgbmFtZS4gSWYgcG9ydCBpcyB1bmRlZmluZWQsXG4gICAqIHdoaWNoIHNob3VsZCBiZSB0aGUgY2FzZSBpZiB0aGVyZSBpcyBvbmx5IG9uZSwgdGhlbiB0aGUgZGVmYXVsdCBwb3J0IGlzXG4gICAqIHVzZWQuXG4gICAqL1xuICBvblJlc3VsdHMobGlzdGVuZXI6IFJlc3VsdE1hcExpc3RlbmVyLCBwb3J0Pzogc3RyaW5nKTogdm9pZCB7XG4gICAgcG9ydCA9IHBvcnQgfHwgREVGQVVMVF9QT1JUX05BTUU7XG4gICAgdGhpcy5saXN0ZW5lcnNbcG9ydF0gPSBsaXN0ZW5lcjtcbiAgfVxufVxuXG5nb29nLmV4cG9ydFN5bWJvbCgnU29sdXRpb24nLCBTb2x1dGlvbik7XG5nb29nLmV4cG9ydFN5bWJvbCgnT3B0aW9uVHlwZScsIE9wdGlvblR5cGUpO1xuIl19
;return exports;});

//third_party/mediapipe/web/solutions/test_harness/test_harness.closure.js
goog.loadModule(function(exports) {'use strict';goog.module('google3.third_party.mediapipe.web.solutions.test_harness.test_harness');
var module = module || { id: 'third_party/mediapipe/web/solutions/test_harness/test_harness.closure.js' };
const tslib_1 = goog.require('google3.third_party.javascript.tslib.tslib');
/**
 *
 * @fileoverview The test harness allows a user to describe a MediaPipe graph,
 * along with the expectations of that graph, and then test those expectations
 * through a test harness (WebDriver, e.g.).  We do this by building up the
 * entire test in a web page based on the specs.
 *
 * Generated from: third_party/mediapipe/web/solutions/test_harness/test_harness.ts
 * @suppress {checkTypes,extraRequire,missingOverride,missingRequire,missingReturn,unusedPrivateMembers,uselessCode} checked by tsc
 */
const tsickle_solutions_api_1 = goog.requireType("google3.third_party.mediapipe.web.solutions.solutions_api");
const solutionsApi = goog.require('google3.third_party.mediapipe.web.solutions.solutions_api');
/**
 * A DIV with class .test will have COMPLETE when the test completes.
 * @type {string}
 */
exports.COMPLETE = 'COMPLETE';
/**
 * A DIV with class .test will have INCOMPLETE until the test completes.
 * @type {string}
 */
exports.INCOMPLETE = 'INCOMPLETE';
/**
 * Describes one item (stream) of a graph.
 * @record
 */
function TestItemEntry() { }
exports.TestItemEntry = TestItemEntry;
/* istanbul ignore if */
if (false) {
    /** @type {string} */
    TestItemEntry.prototype.name;
    /** @type {string} */
    TestItemEntry.prototype.type;
    /** @type {(undefined|number)} */
    TestItemEntry.prototype.tolerance;
    /** @type {(undefined|string)} */
    TestItemEntry.prototype.src;
    /** @type {(undefined|*)} */
    TestItemEntry.prototype.data;
    /** @type {(undefined|string)} */
    TestItemEntry.prototype.dataFile;
    /** @type {(undefined|string)} */
    TestItemEntry.prototype.dataPattern;
    /** @type {(undefined|!HTMLImageElement|!HTMLDivElement)} */
    TestItemEntry.prototype.element;
    /** @type {(undefined|function(): *)} */
    TestItemEntry.prototype.getValue;
    /** @type {(undefined|function(*): !Promise<void>)} */
    TestItemEntry.prototype.render;
    /** @type {(undefined|function(!TestItemEntry): !Promise<(undefined|string)>)} */
    TestItemEntry.prototype.validate;
}
/**
 * Supports extra options for each test case.
 * @record
 */
function TestCaseOptions() { }
exports.TestCaseOptions = TestCaseOptions;
/* istanbul ignore if */
if (false) {
    /** @type {(undefined|number)} */
    TestCaseOptions.prototype.warmup;
}
/**
 * Describes a test.
 * TODO(mhays): Collapse 'outputs' into 'expects'.
 * @record
 */
function TestCase() { }
exports.TestCase = TestCase;
/* istanbul ignore if */
if (false) {
    /** @type {!Array<!TestItemEntry>} */
    TestCase.prototype.inputs;
    /** @type {!Array<!TestItemEntry>} */
    TestCase.prototype.expects;
    /** @type {!Array<!TestItemEntry>} */
    TestCase.prototype.outputs;
    /** @type {(undefined|!TestCaseOptions)} */
    TestCase.prototype.options;
}
/**
 * Desribes a complete test that will be run by a harness.
 * @record
 */
function TestConfig() { }
exports.TestConfig = TestConfig;
/* istanbul ignore if */
if (false) {
    /** @type {string} */
    TestConfig.prototype.name;
    /** @type {(undefined|string)} */
    TestConfig.prototype.binarypb;
    /** @type {(undefined|!tsickle_solutions_api_1.SolutionConfig)} */
    TestConfig.prototype.solution;
    /** @type {(undefined|!TestCase)} */
    TestConfig.prototype.test;
    /** @type {(undefined|string)} */
    TestConfig.prototype.solutionUrl;
    /** @type {(undefined|string)} */
    TestConfig.prototype.testUrl;
}
/**
 * Gives the message queue a chance to empty. Run this if you are depenedent on
 * changes propogating into the DOM.
 * @return {!Promise<void>}
 * @this {*}
 */
function flushDom() {
    return new Promise((/**
     * @param {function((void|!PromiseLike<void>)): void} resolve
     * @return {void}
     */
    (resolve) => {
        setTimeout((/**
         * @return {!Promise<void>}
         */
        () => tslib_1.__awaiter(this, void 0, void 0, function* () {
            resolve();
        })), 0);
    }));
}
/**
 * Defines the solution under test.
 */
class TestSolution {
    /**
     * @param {!tsickle_solutions_api_1.SolutionConfig} config
     */
    constructor(config) {
        this.solution = new solutionsApi.Solution(config);
    }
    /**
     * Shuts down the object. Call before creating a new instance.
     * @return {!Promise<void>}
     */
    close() {
        this.solution.close();
        return Promise.resolve();
    }
    /**
     * Initializes the solution. This includes loading ML models and mediapipe
     * configurations, as well as setting up potential listeners for metadata. If
     * `initialize` is not called manually, then it will be called the first time
     * the developer calls `send`.
     * @return {!Promise<!TestSolution>}
     */
    initialize() {
        return tslib_1.__awaiter(this, void 0, void 0, function* () {
            yield this.solution.initialize();
            return this;
        });
    }
    /**
     * Sends inputs to the solution. The developer can await the results, which
     * resolves when the graph and any listeners have completed.
     * @param {!tsickle_solutions_api_1.InputMap} inputs
     * @return {!Promise<void>}
     */
    send(inputs) {
        return tslib_1.__awaiter(this, void 0, void 0, function* () {
            yield this.solution.send(inputs);
        });
    }
    /**
     * Registers a single callback that will carry any results that occur
     * after calling Send().
     * @param {function(!tsickle_solutions_api_1.ResultMap): (void|!Promise<void>)} listener
     * @return {void}
     */
    onResults(listener) {
        this.solution.onResults(listener);
    }
}
exports.TestSolution = TestSolution;
/* istanbul ignore if */
if (false) {
    /**
     * @type {!tsickle_solutions_api_1.Solution}
     * @private
     */
    TestSolution.prototype.solution;
}
/**
 * Used to store data into the textContent of a DIV tag via JSON.stringigy.
 *
 * Solves a disparity between primitives and objects.  JSON.stringify does not
 * work on number | string | boolean, but if we assign it to the data field of
 * an object, then we can use JSON.stringify normally..
 * @record
 */
function DataHolder() { }
/* istanbul ignore if */
if (false) {
    /** @type {*} */
    DataHolder.prototype.data;
}
/**
 * Create a string representation of a value. Primitives will be rendered
 * through string interpolation, objects will be rendered via JSON.stringify.
 * @param {*} value
 * @return {string}
 */
function displayValue(value) {
    if (value instanceof Object) {
        return JSON.stringify(value);
    }
    return `${value}`;
}
/**
 * Renders a graph's image buffer into an HTMLImageElement.
 * @param {!HTMLImageElement} image
 * @param {!HTMLCanvasElement} buffer
 * @return {!Promise<void>}
 */
function renderImage(image, buffer) {
    /** @type {!HTMLCanvasElement} */
    const canvas = document.createElement('canvas');
    /** @type {!CanvasRenderingContext2D} */
    const canvasCtx = (/** @type {!CanvasRenderingContext2D} */ (canvas.getContext('2d')));
    image.width = canvas.width = buffer.width;
    image.height = canvas.height = buffer.height;
    canvasCtx.clearRect(0, 0, buffer.width, buffer.height);
    canvasCtx.drawImage(buffer, 0, 0, buffer.width, buffer.height);
    /** @type {!HTMLImageElement} */
    const imageLocal = image;
    return new Promise((/**
     * @param {function((void|!PromiseLike<void>)): void} resolve
     * @return {void}
     */
    (resolve) => {
        imageLocal.onload = (/**
         * @return {void}
         */
        () => {
            resolve();
        });
        imageLocal.src = canvas.toDataURL();
    }));
}
/**
 * Returns the image data of an HTMLImageElement (which we will access as a
 * data array).
 * @param {!HTMLImageElement} image
 * @return {!ImageData}
 */
function getImageData(image) {
    /** @type {!HTMLCanvasElement} */
    const canvasFrom = document.createElement('canvas');
    canvasFrom.width = image.width;
    canvasFrom.height = image.height;
    /** @type {!CanvasRenderingContext2D} */
    const ctx = (/** @type {!CanvasRenderingContext2D} */ (canvasFrom.getContext('2d')));
    ctx.drawImage(image, 0, 0, image.width, image.height);
    return ctx.getImageData(0, 0, canvasFrom.width, canvasFrom.height);
}
/**
 * Returns undefined if two HTMLImageElements hold the same image within a
 * %-tolerance. If they don't match, an error message will be returned.
 * @param {string} stream
 * @param {!HTMLImageElement} image1
 * @param {!HTMLImageElement} image2
 * @param {number} tolerance
 * @return {!Promise<(undefined|string)>}
 */
function validateImage(stream, image1, image2, tolerance) {
    return new Promise((/**
     * @param {function((undefined|string|!PromiseLike<(undefined|string)>)): void} resolve
     * @return {void}
     */
    (resolve) => {
        if (image1.width !== image2.width || image1.height !== image2.height) {
            resolve(`Image on stream ${stream}: Dimensions don't match. FOUND: ${image1.width}x${image1.height}, EXPECTED: ${image2.width}x${image2.height}`);
        }
        /** @type {!Uint8ClampedArray} */
        const data1 = getImageData(image1).data;
        /** @type {!Uint8ClampedArray} */
        const data2 = getImageData(image2).data;
        /** @type {number} */
        let diff = 0;
        for (let i = 0; i < data1.length / 4; i++) {
            diff += Math.abs(data1[4 * i + 0] - data2[4 * i + 0]) / 255;
            diff += Math.abs(data1[4 * i + 1] - data2[4 * i + 1]) / 255;
            diff += Math.abs(data1[4 * i + 2] - data2[4 * i + 2]) / 255;
        }
        /** @type {number} */
        const normDiff = 100 * diff / (image1.width * image1.height * 3);
        if (normDiff > tolerance) {
            resolve(`Image on stream ${stream}: Failed within a tolerance of ${tolerance} (was ${normDiff}).`);
        }
        resolve(undefined);
    }));
}
/**
 * @param {!TestItemEntry} item
 * @param {!HTMLDivElement} appendTo
 * @return {void}
 */
function attachLabel(item, appendTo) {
    /** @type {!HTMLDivElement} */
    const label = document.createElement('div');
    label.classList.add('stream');
    label.textContent = `Stream: ${item.name}`;
    appendTo.appendChild(label);
}
/**
 * Given a test item that represents an image, creates a DOM element to hold
 * everything. Also modifies TestItemEntry with methods to manipulate this
 * element.
 * @param {!TestItemEntry} item
 * @param {!HTMLDivElement} appendTo
 * @return {!Promise<void>}
 * @this {*}
 */
function createImageItem(item, appendTo) {
    attachLabel(item, appendTo);
    /** @type {!HTMLImageElement} */
    const image = item.element = document.createElement('img');
    image.classList.add(item.name);
    appendTo.appendChild(image);
    item.getValue = (/**
     * @return {(undefined|!HTMLImageElement|!HTMLDivElement)}
     */
    () => item.element);
    item.render = (/**
     * @param {*} result
     * @return {!Promise<void>}
     */
    (result) => tslib_1.__awaiter(this, void 0, void 0, function* () {
        yield renderImage(image, (/** @type {!HTMLCanvasElement} */ (result)));
    }));
    item.validate = (/**
     * @param {!TestItemEntry} against
     * @return {!Promise<(undefined|string)>}
     */
    (against) => tslib_1.__awaiter(this, void 0, void 0, function* () {
        return validateImage(item.name, (/** @type {!HTMLImageElement} */ ((/** @type {(!HTMLImageElement|!HTMLDivElement)} */ (item.element)))), (/** @type {!HTMLImageElement} */ ((/** @type {(!HTMLImageElement|!HTMLDivElement)} */ (against.element)))), against['tolerance'] || 0);
    }));
    /** @type {(undefined|string)} */
    const imageSrc = item.src;
    if (imageSrc) {
        return new Promise((/**
         * @param {function((void|!PromiseLike<void>)): void} resolve
         * @param {function(?=): void} reject
         * @return {void}
         */
        (resolve, reject) => {
            image.onload = (/**
             * @return {!Promise<void>}
             */
            () => tslib_1.__awaiter(this, void 0, void 0, function* () {
                resolve();
            }));
            image.onerror = (/**
             * @return {?}
             */
            () => {
                throw new Error(`Image ${imageSrc} did not load correctly.`);
            });
            image.src = imageSrc;
        }));
    }
    return Promise.resolve();
}
/**
 * Given a test item that represents an object or primitive, creates a DOM
 * element to hold everything. Also modifies TestItemEntry with methods to
 * manipulate this element.
 * @param {!TestItemEntry} item
 * @param {!HTMLDivElement} appendTo
 * @return {!Promise<void>}
 * @this {*}
 */
function createObjectItem(item, appendTo) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        attachLabel(item, appendTo);
        /** @type {!HTMLDivElement} */
        const element = item.element = document.createElement('div');
        item.getValue = (/**
         * @return {*}
         */
        () => {
            if (element.textContent) {
                return ((/** @type {!DataHolder} */ (JSON.parse(element.textContent)))).data;
            }
            else {
                return undefined;
            }
        });
        item.render = (/**
         * @param {*} result
         * @return {!Promise<void>}
         */
        (result) => tslib_1.__awaiter(this, void 0, void 0, function* () {
            /** @type {{data: !Object}} */
            const buffer = { data: (/** @type {!Object} */ (result)) };
            element.textContent = JSON.stringify(buffer);
        }));
        item.validate = (/**
         * @param {!TestItemEntry} against
         * @return {(!Promise<undefined>|!Promise<string>)}
         */
        (against) => {
            /** @type {*} */
            const itemValue = (/** @type {function(): *} */ (item.getValue))();
            /** @type {*} */
            const againstValue = (/** @type {function(): *} */ (against.getValue))();
            if (against['dataPattern']) {
                /** @type {!RegExp} */
                const re = new RegExp(against['dataPattern']);
                if (re.test(displayValue(itemValue))) {
                    return Promise.resolve(undefined);
                }
                return Promise.resolve(`Object on stream ${item.name} failed pattern ${against['dataPattern']}. FOUND: ${displayValue(itemValue)}`);
            }
            else if (itemValue === againstValue) {
                return Promise.resolve(undefined);
            }
            return Promise.resolve(`Object on stream ${item.name} failed. FOUND: ${displayValue(itemValue)}, EXPECTED: ${displayValue(againstValue)}`);
        });
        element.classList.add(item.name);
        if (item.dataFile) {
            element.textContent = yield (yield fetch(item.dataFile)).text();
        }
        else if (item.dataPattern) {
            element.textContent = JSON.stringify({ data: item.data });
        }
        else if (item.data) {
            element.textContent = JSON.stringify({ data: item.data });
        }
        appendTo.appendChild(element);
        return Promise.resolve();
    });
}
/**
 * Creates all of the test item entries for each item entry provided.
 * @param {!Array<!TestItemEntry>} items
 * @param {!HTMLDivElement} appendTo
 * @return {!Promise<void>}
 * @this {*}
 */
function createWorkspaceItems(items, appendTo) {
    return tslib_1.__awaiter(this, void 0, void 0, function* () {
        for (const item of items) {
            if (item.type === 'video') {
                yield createImageItem(item, appendTo);
            }
            else {
                yield createObjectItem(item, appendTo);
            }
        }
    });
}
/**
 * Connects a test harness up to a specified root node (usually document.body).
 * Call `initialize` to set up the DOM.
 *
 * Usage:
 * 1. Place the JSON.stringify of a test configuration into an input element
 *    with the class 'test' and then click on a button with the class 'run-test'
 *    and wait for the div tag with the class '.test-complete' to show the text
 *    'COMPLETE'.
 */
class TestHarness {
    /**
     * @private
     * @param {!tsickle_solutions_api_1.ResultMap} results
     * @param {!TestConfig} config
     * @return {!Promise<void>}
     */
    onResult(results, config) {
        return tslib_1.__awaiter(this, void 0, void 0, function* () {
            /** @type {!Array<string>} */
            const errors = [];
            for (const output of (/** @type {!TestCase} */ (config['test']))['outputs']) {
                /** @type {string} */
                const itemName = output['name'];
                yield (/** @type {function(*): !Promise<void>} */ (output.render))(results[itemName]);
                for (const expect of (/** @type {!TestCase} */ (config['test']))['expects']) {
                    if (expect['name'] === itemName) {
                        /** @type {(undefined|string)} */
                        const errorMsg = yield (/** @type {function(!TestItemEntry): !Promise<(undefined|string)>} */ (output.validate))(expect);
                        if (errorMsg) {
                            errors.push(errorMsg);
                        }
                        break;
                    }
                }
            }
            this.errorsElement.textContent = errors.join('\n');
            this.testCompleteElement.textContent = exports.COMPLETE;
        });
    }
    /**
     * Executes the test based on the json instructions sitting in testElement.
     * See `onResult` for how we leave data for the test harness to read.
     * @param {!TestConfig} config
     * @return {!Promise<void>}
     */
    run(config) {
        return tslib_1.__awaiter(this, void 0, void 0, function* () {
            if (config['solutionUrl']) {
                /** @type {?} */
                const solutionJson = yield (yield fetch(config['solutionUrl'])).json();
                config['solution'] = (/** @type {!tsickle_solutions_api_1.SolutionConfig} */ (solutionJson));
            }
            if (config['testUrl']) {
                /** @type {?} */
                const testJson = yield (yield fetch(config['testUrl'])).json();
                config['test'] = (/** @type {!TestCase} */ (testJson));
            }
            if (config['binarypb']) {
                (/** @type {!tsickle_solutions_api_1.SolutionConfig} */ (config['solution']))['graph'] = { 'url': config['binarypb'] };
            }
            /** @type {string} */
            const testName = config['name'];
            (/** @type {!tsickle_solutions_api_1.SolutionConfig} */ (config['solution']))['files'] = (/** @type {!tsickle_solutions_api_1.SolutionConfig} */ (config['solution']))['files'] || [];
            (/** @type {!tsickle_solutions_api_1.SolutionConfig} */ (config['solution']))['files'] = [
                ...(/** @type {!tsickle_solutions_api_1.SolutionConfig} */ (config['solution']))['files'],
                { url: `${testName}_web_solution_packed_assets_loader.js` },
                { simd: false, url: `${testName}_web_solution_wasm_bin.js` },
                { simd: true, url: `${testName}_web_solution_simd_wasm_bin.js` }
            ];
            /** @type {!TestSolution} */
            const solution = yield new TestSolution((/** @type {!tsickle_solutions_api_1.SolutionConfig} */ (config['solution']))).initialize();
            yield createWorkspaceItems((/** @type {!TestCase} */ (config['test']))['inputs'], this.inputs);
            yield createWorkspaceItems((/** @type {!TestCase} */ (config['test']))['expects'], this.expects);
            yield createWorkspaceItems((/** @type {!TestCase} */ (config['test']))['outputs'], this.outputs);
            yield flushDom();
            /** @type {!Object<string,*>} */
            const inputs = {};
            for (const input of (/** @type {!TestCase} */ (config['test']))['inputs']) {
                inputs[input.name] = (/** @type {function(): *} */ (input.getValue))();
            }
            this.testReadyElement.textContent = exports.COMPLETE;
            if ((/** @type {!TestCase} */ (config['test']))['options'] && (/** @type {!TestCase} */ (config['test']))['options']['warmup']) {
                /** @type {number} */
                const warmup = (/** @type {!TestCase} */ (config['test']))['options']['warmup'];
                for (let loop = 0; loop < warmup; loop++) {
                    yield solution.send(inputs);
                }
            }
            solution.onResults((/**
             * @param {!tsickle_solutions_api_1.ResultMap} data
             * @return {void}
             */
            (data) => {
                this.onResult(data, config);
            }));
            yield solution.send(inputs);
        });
    }
    /**
     * Sets up the test and should be called before run is called. By design,
     * run will be called by simulating a click on a button with class `run-test`.
     * @return {!Promise<void>}
     */
    initialize() {
        return tslib_1.__awaiter(this, void 0, void 0, function* () {
            this.testElement = (/** @type {!HTMLTextAreaElement} */ (document.querySelector('.test')));
            this.runTestButton =
                (/** @type {!HTMLButtonElement} */ (document.querySelector('.run-test')));
            this.testCompleteElement =
                (/** @type {!HTMLDivElement} */ (document.querySelector('.test-complete')));
            this.errorsElement =
                (/** @type {!HTMLTextAreaElement} */ (document.querySelector('.errors')));
            this.testReadyElement =
                (/** @type {!HTMLDivElement} */ (document.querySelector('.test-ready')));
            this.outputs = (/** @type {!HTMLDivElement} */ (document.querySelector('.outputs')));
            this.expects = (/** @type {!HTMLDivElement} */ (document.querySelector('.expects')));
            this.inputs = (/** @type {!HTMLDivElement} */ (document.querySelector('.inputs')));
            this.runTestButton.onclick = (/**
             * @return {void}
             */
            () => {
                this.run((/** @type {!TestConfig} */ (JSON.parse(this.testElement.value))));
            });
            yield flushDom();
        });
    }
}
exports.TestHarness = TestHarness;
/* istanbul ignore if */
if (false) {
    /**
     * @type {!HTMLTextAreaElement}
     * @private
     */
    TestHarness.prototype.testElement;
    /**
     * @type {!HTMLTextAreaElement}
     * @private
     */
    TestHarness.prototype.errorsElement;
    /**
     * @type {!HTMLButtonElement}
     * @private
     */
    TestHarness.prototype.runTestButton;
    /**
     * @type {!HTMLDivElement}
     * @private
     */
    TestHarness.prototype.testCompleteElement;
    /**
     * @type {!HTMLDivElement}
     * @private
     */
    TestHarness.prototype.testReadyElement;
    /**
     * @type {!HTMLDivElement}
     * @private
     */
    TestHarness.prototype.inputs;
    /**
     * @type {!HTMLDivElement}
     * @private
     */
    TestHarness.prototype.expects;
    /**
     * @type {!HTMLDivElement}
     * @private
     */
    TestHarness.prototype.outputs;
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoidGVzdF9oYXJuZXNzLmpzIiwic291cmNlUm9vdCI6IiIsInNvdXJjZXMiOlsiLi4vLi4vLi4vLi4vLi4vLi4vLi4vLi4vdGhpcmRfcGFydHkvbWVkaWFwaXBlL3dlYi9zb2x1dGlvbnMvdGVzdF9oYXJuZXNzL3Rlc3RfaGFybmVzcy50cyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiOzs7Ozs7Ozs7Ozs7OztBQU9BLCtGQUEwRjs7Ozs7QUFHN0UsUUFBQSxRQUFRLEdBQUcsVUFBVTs7Ozs7QUFFckIsUUFBQSxVQUFVLEdBQUcsWUFBWTs7Ozs7QUFHdEMsNEJBWUM7Ozs7O0lBWEMsNkJBQWE7O0lBQ2IsNkJBQXVCOztJQUN2QixrQ0FBbUI7O0lBQ25CLDRCQUFhOztJQUNiLDZCQUFVOztJQUNWLGlDQUFrQjs7SUFDbEIsb0NBQXFCOztJQUNyQixnQ0FBMEM7O0lBQzFDLGlDQUF5Qjs7SUFDekIsK0JBQTBDOztJQUMxQyxpQ0FBaUU7Ozs7OztBQUluRSw4QkFFQzs7Ozs7SUFEQyxpQ0FBa0I7Ozs7Ozs7QUFPcEIsdUJBTUM7Ozs7O0lBSkMsMEJBQTBCOztJQUMxQiwyQkFBMkI7O0lBQzNCLDJCQUEyQjs7SUFDM0IsMkJBQTRCOzs7Ozs7QUFJOUIseUJBZ0JDOzs7OztJQWRDLDBCQUFhOztJQUdiLDhCQUFrQjs7SUFHbEIsOEJBQXVDOztJQUN2QywwQkFBZ0I7O0lBR2hCLGlDQUFxQjs7SUFHckIsNkJBQWlCOzs7Ozs7OztBQU9uQixTQUFTLFFBQVE7SUFDZixPQUFPLElBQUksT0FBTzs7OztJQUFDLENBQUMsT0FBTyxFQUFFLEVBQUU7UUFDN0IsVUFBVTs7O1FBQUMsR0FBUyxFQUFFO1lBQ3BCLE9BQU8sRUFBRSxDQUFDO1FBQ1osQ0FBQyxDQUFBLEdBQUUsQ0FBQyxDQUFDLENBQUM7SUFDUixDQUFDLEVBQUMsQ0FBQztBQUNMLENBQUM7Ozs7QUFLRCxNQUFhLFlBQVk7Ozs7SUFHdkIsWUFBWSxNQUFtQztRQUM3QyxJQUFJLENBQUMsUUFBUSxHQUFHLElBQUksWUFB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QUMsU0FBUyxDQUFDLEVBQWtCLENBQUM7WUFDbEUsSUFBSSxDQUFDLGFBQWEsQ0FBQyxPQUFPOzs7WUFBRyxHQUFHLEVBQUU7Z0JBQ2hDLElBQUksQ0FBQyxHQUFHLENBQUMsNkJBQUEsSUFBSSxDQUFDLEtBQUssQ0FBQyxJQUFJLENBQUMsV0FBVyxDQUFDLEtBQUssQ0FBQyxFQUFjLENBQUMsQ0FBQztZQUM3RCxDQUFDLENBQUEsQ0FBQztZQUNGLE1BQU0sUUFBUSxFQUFFLENBQUM7UUFDbkIsQ0FBQztLQUFBO0NBQ0Y7QUFyR0Qsa0NBcUdDOzs7Ozs7O0lBcEdDLGtDQUEwQzs7Ozs7SUFDMUMsb0NBQTRDOzs7OztJQUM1QyxvQ0FBMEM7Ozs7O0lBQzFDLDBDQUE2Qzs7Ozs7SUFDN0MsdUNBQTBDOzs7OztJQUMxQyw2QkFBZ0M7Ozs7O0lBQ2hDLDhCQUFpQzs7Ozs7SUFDakMsOEJBQWlDIiwic291cmNlc0NvbnRlbnQiOlsiLyoqXG4gKiBAZmlsZW92ZXJ2aWV3IFRoZSB0ZXN0IGhhcm5lc3MgYWxsb3dzIGEgdXNlciB0byBkZXNjcmliZSBhIE1lZGlhUGlwZSBncmFwaCxcbiAqIGFsb25nIHdpdGggdGhlIGV4cGVjdGF0aW9ucyBvZiB0aGF0IGdyYXBoLCBhbmQgdGhlbiB0ZXN0IHRob3NlIGV4cGVjdGF0aW9uc1xuICogdGhyb3VnaCBhIHRlc3QgaGFybmVzcyAoV2ViRHJpdmVyLCBlLmcuKS4gIFdlIGRvIHRoaXMgYnkgYnVpbGRpbmcgdXAgdGhlXG4gKiBlbnRpcmUgdGVzdCBpbiBhIHdlYiBwYWdlIGJhc2VkIG9uIHRoZSBzcGVjcy5cbiAqL1xuXG5pbXBvcnQgKiBhcyBzb2x1dGlvbnNBcGkgZnJvbSAnZ29vZ2xlMy90aGlyZF9wYXJ0eS9tZWRpYXBpcGUvd2ViL3NvbHV0aW9ucy9zb2x1dGlvbnNfYXBpJztcblxuLyoqIEEgRElWIHdpdGggY2xhc3MgLnRlc3Qgd2lsbCBoYXZlIENPTVBMRVRFIHdoZW4gdGhlIHRlc3QgY29tcGxldGVzLiAqL1xuZXhwb3J0IGNvbnN0IENPTVBMRVRFID0gJ0NPTVBMRVRFJztcbi8qKiBBIERJViB3aXRoIGNsYXNzIC50ZXN0IHdpbGwgaGF2ZSBJTkNPTVBMRVRFIHVudGlsIHRoZSB0ZXN0IGNvbXBsZXRlcy4gKi9cbmV4cG9ydCBjb25zdCBJTkNPTVBMRVRFID0gJ0lOQ09NUExFVEUnO1xuXG4vKiogRGVzY3JpYmVzIG9uZSBpdGVtIChzdHJlYW0pIG9mIGEgZ3JhcGguICovXG5leHBvcnQgaW50ZXJmYWNlIFRlc3RJdGVtRW50cnkge1xuICBuYW1lOiBzdHJpbmc7XG4gIHR5cGU6ICd2aWRlbyd8J29iamVjdCc7XG4gIHRvbGVyYW5jZT86IG51bWJlcjtcbiAgc3JjPzogc3RyaW5nO1xuICBkYXRhPzoge307XG4gIGRhdGFGaWxlPzogc3RyaW5nO1xuICBkYXRhUGF0dGVybj86IHN0cmluZztcbiAgZWxlbWVudD86IEhUTUxEaXZFbGVtZW50fEhUTUxJbWFnZUVsZW1lbnQ7XG4gIGdldFZhbHVlPzogKCkgPT4gdW5rbm93bjtcbiAgcmVuZGVyPzogKGRhdGE6IHVua25vd24pID0+IFByb21pc2U8dm9pZD47XG4gIHZhbGlkYXRlPzogKGFnYWluc3Q6IFRlc3RJdGVtRW50cnkpID0+IFByb21pc2U8c3RyaW5nfHVuZGVmaW5lZD47XG59XG5cbi8qKiBTdXBwb3J0cyBleHRyYSBvcHRpb25zIGZvciBlYWNoIHRlc3QgY2FzZS4gKi9cbmV4cG9ydCBpbnRlcmZhY2UgVGVzdENhc2VPcHRpb25zIHtcbiAgJ3dhcm11cCc/OiBudW1iZXI7XG59XG5cbi8qKlxuICogRGVzY3JpYmVzIGEgdGVzdC5cbiAqIFRPRE8obWhheXMpOiBDb2xsYXBzZSAnb3V0cHV0cycgaW50byAnZXhwZWN0cycuXG4gKi9cbmV4cG9ydCBpbnRlcmZhY2UgVGVzdENhc2Uge1xuICAvLyBUaW1lIHRvIGFsbG93IHRoZSB0ZXN0IHRvIHJ1biBiZWZvcmUgZmFpbGluZy4gSW4gbWlsbGlzZWNvbmRzLlxuICAnaW5wdXRzJzogVGVzdEl0ZW1FbnRyeVtdO1xuICAnZXhwZWN0cyc6IFRlc3RJdGVtRW50cnlbXTtcbiAgJ291dHB1dHMnOiBUZXN0SXRlbUVudHJ5W107XG4gICdvcHRpb25zJz86IFRlc3RDYXNlT3B0aW9ucztcbn1cblxuLyoqIERlc3JpYmVzIGEgY29tcGxldGUgdGVzdCB0aGF0IHdpbGwgYmUgcnVuIGJ5IGEgaGFybmVzcy4gKi9cbmV4cG9ydCBpbnRlcmZhY2UgVGVzdENvbmZpZyB7XG4gIC8vIE5hbWUgb2YgdGhlIHRlc3QsIHVzZWQgdG8gY3JlYXRlIGEgZmV3IGZpbGUgbmFtZXMgd2hlbiBsb2FkaW5nLlxuICBuYW1lOiBzdHJpbmc7XG4gIC8vIE5hbWUgb2YgdGhlIGJpbmFyeXBiIGZpbGUuIElmIHNwZWNpZmllZCwgaXQgb3ZlcndyaXRlcyB0aGUgb25lIGluIHRoZVxuICAvLyBncmFwaC5qc29uLlxuICBiaW5hcnlwYj86IHN0cmluZztcbiAgLy8gVGhlIFNvbHV0aW9uIGNvbmZpZyBtdXN0IHN1cnZpdmUgSlNPTi5zdHJpbmdpZnkgLT4gSlNPTi5wYXJzZSBhcyB3ZSBhcmVcbiAgLy8gc2VuZGluZyB0aGlzIG9iamVjdCBhcyBhIG1lc3NhZ2UgKG1vcmUgb3IgbGVzcykuXG4gIHNvbHV0aW9uPzogc29sdXRpb25zQXBpLlNvbHV0aW9uQ29uZmlnO1xuICB0ZXN0PzogVGVzdENhc2U7XG4gIC8vIElmIHN1cHBsaWVkLCB0aGUgaGFybmVzcyB3aWxsIHJlYWQgdGhlIEpTT04gZnJvbSB0aGUgdXJsIGFuZCBwdXQgaXQgaW50b1xuICAvLyBzb2x1dGlvbi5cbiAgc29sdXRpb25Vcmw/OiBzdHJpbmc7XG4gIC8vIElmIHN1cHBsaWVkLCB0aGUgaGFybmVzcyB3aWxsIHJlYWQgdGhlIEpTT04gZnJvbSB0aGUgdXJsIGFuZCBwdXQgaXQgaW50b1xuICAvLyB0ZXN0LlxuICB0ZXN0VXJsPzogc3RyaW5nO1xufVxuXG4vKipcbiAqIEdpdmVzIHRoZSBtZXNzYWdlIHF1ZXVlIGEgY2hhbmNlIHRvIGVtcHR5LiBSdW4gdGhpcyBpZiB5b3UgYXJlIGRlcGVuZWRlbnQgb25cbiAqIGNoYW5nZXMgcHJvcG9nYXRpbmcgaW50byB0aGUgRE9NLlxuICovXG5mdW5jdGlvbiBmbHVzaERvbSgpOiBQcm9taXNlPHZvaWQ+IHtcbiAgcmV0dXJuIG5ldyBQcm9taXNlKChyZXNvbHZlKSA9PiB7XG4gICAgc2V0VGltZW91dChhc3luYyAoKSA9PiB7XG4gICAgICByZXNvbHZlKCk7XG4gICAgfSwgMCk7XG4gIH0pO1xufVxuXG4vKipcbiAqIERlZmluZXMgdGhlIHNvbHV0aW9uIHVuZGVyIHRlc3QuXG4gKi9cbmV4cG9ydCBjbGFzcyBUZXN0U29sdXRpb24ge1xuICBwcml2YXRlIHJlYWRvbmx5IHNvbHV0aW9uOiBzb2x1dGlvbnNBcGkuU29sdXRpb247XG5cbiAgY29uc3RydWN0b3IoY29uZmlnOiBzb2x1dGlvbnNBcGkuU29sdXRpb25Db25maWcpIHtcbiAgICB0aGlzLnNvbHV0aW9uID0gbmV3IHNvbHV0aW9uc0FwaS5Tb2x1dGlvbihjb25maWcpO1xuICB9XG5cbiAgLyoqXG4gICAqIFNodXRzIGRvd24gdGhlIG9iamVjdC4gQ2FsbCBiZWZvcmUgY3JlYXRpbmcgYSBuZXcgaW5zdGFuY2UuXG4gICAqL1xuICBjbG9zZSgpOiBQcm9taXNlPHZvaWQ+IHtcbiAgICB0aGlzLnNvbHV0aW9uLmNsb3NlKCk7XG4gICAgcmV0dXJuIFByb21pc2UucmVzb2x2ZSgpO1xuICB9XG5cbiAgLyoqXG4gICAqIEluaXRpYWxpemVzIHRoZSBzb2x1dGlvbi4gVGhpcyBpbmNsdWRlcyBsb2FkaW5nIE1MIG1vZGVscyBhbmQgbWVkaWFwaXBlXG4gICAqIGNvbmZpZ3VyYXRpb25zLCBhcyB3ZWxsIGFzIHNldHRpbmcgdXAgcG90ZW50aWFsIGxpc3RlbmVycyBmb3IgbWV0YWRhdGEuIElmXG4gICAqIGBpbml0aWFsaXplYCBpcyBub3QgY2FsbGVkIG1hbnVhbGx5LCB0aGVuIGl0IHdpbGwgYmUgY2FsbGVkIHRoZSBmaXJzdCB0aW1lXG4gICAqIHRoZSBkZXZlbG9wZXIgY2FsbHMgYHNlbmRgLlxuICAgKi9cbiAgYXN5bmMgaW5pdGlhbGl6ZSgpOiBQcm9taXNlPFRlc3RTb2x1dGlvbj4ge1xuICAgIGF3YWl0IHRoaXMuc29sdXRpb24uaW5pdGlhbGl6ZSgpO1xuICAgIHJldHVybiB0aGlzO1xuICB9XG5cbiAgLyoqXG4gICAqIFNlbmRzIGlucHV0cyB0byB0aGUgc29sdXRpb24uIFRoZSBkZXZlbG9wZXIgY2FuIGF3YWl0IHRoZSByZXN1bHRzLCB3aGljaFxuICAgKiByZXNvbHZlcyB3aGVuIHRoZSBncmFwaCBhbmQgYW55IGxpc3RlbmVycyBoYXZlIGNvbXBsZXRlZC5cbiAgICovXG4gIGFzeW5jIHNlbmQoaW5wdXRzOiBzb2x1dGlvbnNBcGkuSW5wdXRNYXApOiBQcm9taXNlPHZvaWQ+IHtcbiAgICBhd2FpdCB0aGlzLnNvbHV0aW9uLnNlbmQoaW5wdXRzKTtcbiAgfVxuXG4gIC8qKlxuICAgKiBSZWdpc3RlcnMgYSBzaW5nbGUgY2FsbGJhY2sgdGhhdCB3aWxsIGNhcnJ5IGFueSByZXN1bHRzIHRoYXQgb2NjdXJcbiAgICogYWZ0ZXIgY2FsbGluZyBTZW5kKCkuXG4gICAqL1xuICBvblJlc3VsdHMobGlzdGVuZXI6IHNvbHV0aW9uc0FwaS5SZXN1bHRNYXBMaXN0ZW5lcik6IHZvaWQge1xuICAgIHRoaXMuc29sdXRpb24ub25SZXN1bHRzKGxpc3RlbmVyKTtcbiAgfVxufVxuXG4vKipcbiAqIFVzZWQgdG8gc3RvcmUgZGF0YSBpbnRvIHRoZSB0ZXh0Q29udGVudCBvZiBhIERJViB0YWcgdmlhIEpTT04uc3RyaW5naWd5LlxuICpcbiAqIFNvbHZlcyBhIGRpc3Bhcml0eSBiZXR3ZWVuIHByaW1pdGl2ZXMgYW5kIG9iamVjdHMuICBKU09OLnN0cmluZ2lmeSBkb2VzIG5vdFxuICogd29yayBvbiBudW1iZXIgfCBzdHJpbmcgfCBib29sZWFuLCBidXQgaWYgd2UgYXNzaWduIGl0IHRvIHRoZSBkYXRhIGZpZWxkIG9mXG4gKiBhbiBvYmplY3QsIHRoZW4gd2UgY2FuIHVzZSBKU09OLnN0cmluZ2lmeSBub3JtYWxseS4uXG4gKi9cbmludGVyZmFjZSBEYXRhSG9sZGVyIHtcbiAgZGF0YTogdW5rbm93bjtcbn1cblxuLyoqXG4gKiBDcmVhdGUgYSBzdHJpbmcgcmVwcmVzZW50YXRpb24gb2YgYSB2YWx1ZS4gUHJpbWl0aXZlcyB3aWxsIGJlIHJlbmRlcmVkXG4gKiB0aHJvdWdoIHN0cmluZyBpbnRlcnBvbGF0aW9uLCBvYmplY3RzIHdpbGwgYmUgcmVuZGVyZWQgdmlhIEpTT04uc3RyaW5naWZ5LlxuICovXG5mdW5jdGlvbiBkaXNwbGF5VmFsdWUodmFsdWU6IHVua25vd24pOiBzdHJpbmcge1xuICBpZiAodmFsdWUgaW5zdGFuY2VvZiBPYmplY3QpIHtcbiAgICByZXR1cm4gSlNPTi5zdHJpbmdpZnkodmFsdWUpO1xuICB9XG4gIHJldHVybiBgJHt2YWx1ZX1gO1xufVxuXG4vKipcbiAqIFJlbmRlcnMgYSBncmFwaCdzIGltYWdlIGJ1ZmZlciBpbnRvIGFuIEhUTUxJbWFnZUVsZW1lbnQuXG4gKi9cbmZ1bmN0aW9uIHJlbmRlckltYWdlKFxuICAgIGltYWdlOiBIVE1MSW1hZ2VFbGVtZW50LCBidWZmZXI6IHNvbHV0aW9uc0FwaS5HcHVCdWZmZXIpOiBQcm9taXNlPHZvaWQ+IHtcbiAgY29uc3QgY2FudmFzID0gZG9jdW1lbnQuY3JlYXRlRWxlbWVudCgnY2FudmFzJyk7XG4gIGNvbnN0IGNhbnZhc0N0eCA9IGNhbnZhcy5nZXRDb250ZXh0KCcyZCcpITtcbiAgaW1hZ2Uud2lkdGggPSBjYW52YXMud2lkdGggPSBidWZmZXIud2lkdGg7XG4gIGltYWdlLmhlaWdodCA9IGNhbnZhcy5oZWlnaHQgPSBidWZmZXIuaGVpZ2h0O1xuICBjYW52YXNDdHguY2xlYXJSZWN0KDAsIDAsIGJ1ZmZlci53aWR0aCwgYnVmZmVyLmhlaWdodCk7XG4gIGNhbnZhc0N0eC5kcmF3SW1hZ2UoYnVmZmVyLCAwLCAwLCBidWZmZXIud2lkdGgsIGJ1ZmZlci5oZWlnaHQpO1xuXG4gIGNvbnN0IGltYWdlTG9jYWwgPSBpbWFnZTtcbiAgcmV0dXJuIG5ldyBQcm9taXNlKChyZXNvbHZlKSA9PiB7XG4gICAgaW1hZ2VMb2NhbC5vbmxvYWQgPSAoKSA9PiB7XG4gICAgICByZXNvbHZlKCk7XG4gICAgfTtcbiAgICBpbWFnZUxvY2FsLnNyYyA9IGNhbnZhcy50b0RhdGFVUkwoKTtcbiAgfSk7XG59XG5cbi8qKlxuICogUmV0dXJucyB0aGUgaW1hZ2UgZGF0YSBvZiBhbiBIVE1MSW1hZ2VFbGVtZW50ICh3aGljaCB3ZSB3aWxsIGFjY2VzcyBhcyBhXG4gKiBkYXRhIGFycmF5KS5cbiAqL1xuZnVuY3Rpb24gZ2V0SW1hZ2VEYXRhKGltYWdlOiBIVE1MSW1hZ2VFbGVtZW50KTogSW1hZ2VEYXRhIHtcbiAgY29uc3QgY2FudmFzRnJvbSA9IGRvY3VtZW50LmNyZWF0ZUVsZW1lbnQoJ2NhbnZhcycpO1xuICBjYW52YXNGcm9tLndpZHRoID0gaW1hZ2Uud2lkdGg7XG4gIGNhbnZhc0Zyb20uaGVpZ2h0ID0gaW1hZ2UuaGVpZ2h0O1xuICBjb25zdCBjdHggPSBjYW52YXNGcm9tLmdldENvbnRleHQoJzJkJykhO1xuICBjdHguZHJhd0ltYWdlKGltYWdlLCAwLCAwLCBpbWFnZS53aWR0aCwgaW1hZ2UuaGVpZ2h0KTtcbiAgcmV0dXJuIGN0eC5nZXRJbWFnZURhdGEoMCwgMCwgY2FudmFzRnJvbS53aWR0aCwgY2FudmFzRnJvbS5oZWlnaHQpO1xufVxuXG4vKipcbiAqIFJldHVybnMgdW5kZWZpbmVkIGlmIHR3byBIVE1MSW1hZ2VFbGVtZW50cyBob2xkIHRoZSBzYW1lIGltYWdlIHdpdGhpbiBhXG4gKiAlLXRvbGVyYW5jZS4gSWYgdGhleSBkb24ndCBtYXRjaCwgYW4gZXJyb3IgbWVzc2FnZSB3aWxsIGJlIHJldHVybmVkLlxuICovXG5mdW5jdGlvbiB2YWxpZGF0ZUltYWdlKFxuICAgIHN0cmVhbTogc3RyaW5nLCBpbWFnZTE6IEhUTUxJbWFnZUVsZW1lbnQsIGltYWdlMjogSFRNTEltYWdlRWxlbWVudCxcbiAgICB0b2xlcmFuY2U6IG51bWJlcik6IFByb21pc2U8c3RyaW5nfHVuZGVmaW5lZD4ge1xuICByZXR1cm4gbmV3IFByb21pc2UoKHJlc29sdmUpID0+IHtcbiAgICBpZiAoaW1hZ2UxLndpZHRoICE9PSBpbWFnZTIud2lkdGggfHwgaW1hZ2UxLmhlaWdodCAhPT0gaW1hZ2UyLmhlaWdodCkge1xuICAgICAgcmVzb2x2ZShgSW1hZ2Ugb24gc3RyZWFtICR7c3RyZWFtfTogRGltZW5zaW9ucyBkb24ndCBtYXRjaC4gRk9VTkQ6ICR7XG4gICAgICAgICAgaW1hZ2UxLndpZHRofXgke2ltYWdlMS5oZWlnaHR9LCBFWFBFQ1RFRDogJHtpbWFnZTIud2lkdGh9eCR7XG4gICAgICAgICAgaW1hZ2UyLmhlaWdodH1gKTtcbiAgICB9XG4gICAgY29uc3QgZGF0YTEgPSBnZXRJbWFnZURhdGEoaW1hZ2UxKS5kYXRhO1xuICAgIGNvbnN0IGRhdGEyID0gZ2V0SW1hZ2VEYXRhKGltYWdlMikuZGF0YTtcblxuICAgIGxldCBkaWZmID0gMDtcbiAgICBmb3IgKGxldCBpID0gMDsgaSA8IGRhdGExLmxlbmd0aCAvIDQ7IGkrKykge1xuICAgICAgZGlmZiArPSBNYXRoLmFicyhkYXRhMVs0ICogaSArIDBdIC0gZGF0YTJbNCAqIGkgKyAwXSkgLyAyNTU7XG4gICAgICBkaWZmICs9IE1hdGguYWJzKGRhdGExWzQgKiBpICsgMV0gLSBkYXRhMls0ICogaSArIDFdKSAvIDI1NTtcbiAgICAgIGRpZmYgKz0gTWF0aC5hYnMoZGF0YTFbNCAqIGkgKyAyXSAtIGRhdGEyWzQgKiBpICsgMl0pIC8gMjU1O1xuICAgIH1cbiAgICBjb25zdCBub3JtRGlmZiA9IDEwMCAqIGRpZmYgLyAoaW1hZ2UxLndpZHRoICogaW1hZ2UxLmhlaWdodCAqIDMpO1xuICAgIGlmIChub3JtRGlmZiA+IHRvbGVyYW5jZSkge1xuICAgICAgcmVzb2x2ZShgSW1hZ2Ugb24gc3RyZWFtICR7c3RyZWFtfTogRmFpbGVkIHdpdGhpbiBhIHRvbGVyYW5jZSBvZiAke1xuICAgICAgICAgIHRvbGVyYW5jZX0gKHdhcyAke25vcm1EaWZmfSkuYCk7XG4gICAgfVxuICAgIHJlc29sdmUodW5kZWZpbmVkKTtcbiAgfSk7XG59XG5cbmZ1bmN0aW9uIGF0dGFjaExhYmVsKGl0ZW06IFRlc3RJdGVtRW50cnksIGFwcGVuZFRvOiBIVE1MRGl2RWxlbWVudCk6IHZvaWQge1xuICBjb25zdCBsYWJlbCA9IGRvY3VtZW50LmNyZWF0ZUVsZW1lbnQoJ2RpdicpO1xuICBsYWJlbC5jbGFzc0xpc3QuYWRkKCdzdHJlYW0nKTtcbiAgbGFiZWwudGV4dENvbnRlbnQgPSBgU3RyZWFtOiAke2l0ZW0ubmFtZX1gO1xuICBhcHBlbmRUby5hcHBlbmRDaGlsZChsYWJlbCk7XG59XG5cbi8qKlxuICogR2l2ZW4gYSB0ZXN0IGl0ZW0gdGhhdCByZXByZXNlbnRzIGFuIGltYWdlLCBjcmVhdGVzIGEgRE9NIGVsZW1lbnQgdG8gaG9sZFxuICogZXZlcnl0aGluZy4gQWxzbyBtb2RpZmllcyBUZXN0SXRlbUVudHJ5IHdpdGggbWV0aG9kcyB0byBtYW5pcHVsYXRlIHRoaXNcbiAqIGVsZW1lbnQuXG4gKi9cbmZ1bmN0aW9uIGNyZWF0ZUltYWdlSXRlbShcbiAgICBpdGVtOiBUZXN0SXRlbUVudHJ5LCBhcHBlbmRUbzogSFRNTERpdkVsZW1lbnQpOiBQcm9taXNlPHZvaWQ+IHtcbiAgYXR0YWNoTGFiZWwoaXRlbSwgYXBwZW5kVG8pO1xuICBjb25zdCBpbWFnZSA9IGl0ZW0uZWxlbWVudCA9IGRvY3VtZW50LmNyZWF0ZUVsZW1lbnQoJ2ltZycpO1xuICBpbWFnZS5jbGFzc0xpc3QuYWRkKGl0ZW0ubmFtZSk7XG4gIGFwcGVuZFRvLmFwcGVuZENoaWxkKGltYWdlKTtcblxuICBpdGVtLmdldFZhbHVlID0gKCkgPT4gaXRlbS5lbGVtZW50O1xuICBpdGVtLnJlbmRlciA9IGFzeW5jIChyZXN1bHQpID0+IHtcbiAgICBhd2FpdCByZW5kZXJJbWFnZShpbWFnZSwgcmVzdWx0IGFzIHNvbHV0aW9uc0FwaS5HcHVCdWZmZXIpO1xuICB9O1xuICBpdGVtLnZhbGlkYXRlID0gYXN5bmMgKGFnYWluc3QpID0+IHtcbiAgICByZXR1cm4gdmFsaWRhdGVJbWFnZShcbiAgICAgICAgaXRlbS5uYW1lLCBpdGVtLmVsZW1lbnQhIGFzIEhUTUxJbWFnZUVsZW1lbnQsXG4gICAgICAgIGFnYWluc3QuZWxlbWVudCEgYXMgSFRNTEltYWdlRWxlbWVudCwgYWdhaW5zdFsndG9sZXJhbmNlJ10gfHwgMCk7XG4gIH07XG5cbiAgY29uc3QgaW1hZ2VTcmMgPSBpdGVtLnNyYztcbiAgaWYgKGltYWdlU3JjKSB7XG4gICAgcmV0dXJuIG5ldyBQcm9taXNlKChyZXNvbHZlLCByZWplY3QpID0+IHtcbiAgICAgIGltYWdlLm9ubG9hZCA9IGFzeW5jICgpID0+IHtcbiAgICAgICAgcmVzb2x2ZSgpO1xuICAgICAgfTtcbiAgICAgIGltYWdlLm9uZXJyb3IgPSAoKSA9PiB7XG4gICAgICAgIHRocm93IG5ldyBFcnJvcihgSW1hZ2UgJHtpbWFnZVNyY30gZGlkIG5vdCBsb2FkIGNvcnJlY3RseS5gKTtcbiAgICAgIH07XG4gICAgICBpbWFnZS5zcmMgPSBpbWFnZVNyYztcbiAgICB9KTtcbiAgfVxuICByZXR1cm4gUHJvbWlzZS5yZXNvbHZlKCk7XG59XG5cbi8qKlxuICogR2l2ZW4gYSB0ZXN0IGl0ZW0gdGhhdCByZXByZXNlbnRzIGFuIG9iamVjdCBvciBwcmltaXRpdmUsIGNyZWF0ZXMgYSBET01cbiAqIGVsZW1lbnQgdG8gaG9sZCBldmVyeXRoaW5nLiBBbHNvIG1vZGlmaWVzIFRlc3RJdGVtRW50cnkgd2l0aCBtZXRob2RzIHRvXG4gKiBtYW5pcHVsYXRlIHRoaXMgZWxlbWVudC5cbiAqL1xuYXN5bmMgZnVuY3Rpb24gY3JlYXRlT2JqZWN0SXRlbShcbiAgICBpdGVtOiBUZXN0SXRlbUVudHJ5LCBhcHBlbmRUbzogSFRNTERpdkVsZW1lbnQpOiBQcm9taXNlPHZvaWQ+IHtcbiAgYXR0YWNoTGFiZWwoaXRlbSwgYXBwZW5kVG8pO1xuICBjb25zdCBlbGVtZW50ID0gaXRlbS5lbGVtZW50ID0gZG9jdW1lbnQuY3JlYXRlRWxlbWVudCgnZGl2Jyk7XG4gIGl0ZW0uZ2V0VmFsdWUgPSAoKSA9PiB7XG4gICAgaWYgKGVsZW1lbnQudGV4dENvbnRlbnQpIHtcbiAgICAgIHJldHVybiAoSlNPTi5wYXJzZShlbGVtZW50LnRleHRDb250ZW50KSBhcyBEYXRhSG9sZGVyKS5kYXRhO1xuICAgIH0gZWxzZSB7XG4gICAgICByZXR1cm4gdW5kZWZpbmVkO1xuICAgIH1cbiAgfTtcbiAgaXRlbS5yZW5kZXIgPSBhc3luYyAocmVzdWx0OiB1bmtub3duKSA9PiB7XG4gICAgY29uc3QgYnVmZmVyID0ge2RhdGE6IHJlc3VsdCBhcyBvYmplY3R9O1xuICAgIGVsZW1lbnQudGV4dENvbnRlbnQgPSBKU09OLnN0cmluZ2lmeShidWZmZXIpO1xuICB9O1xuICBpdGVtLnZhbGlkYXRlID0gKGFnYWluc3Q6IFRlc3RJdGVtRW50cnkpID0+IHtcbiAgICBjb25zdCBpdGVtVmFsdWUgPSBpdGVtLmdldFZhbHVlISgpO1xuICAgIGNvbnN0IGFnYWluc3RWYWx1ZSA9IGFnYWluc3QuZ2V0VmFsdWUhKCk7XG4gICAgaWYgKGFnYWluc3RbJ2RhdGFQYXR0ZXJuJ10pIHtcbiAgICAgIGNvbnN0IHJlID0gbmV3IFJlZ0V4cChhZ2FpbnN0WydkYXRhUGF0dGVybiddKTtcbiAgICAgIGlmIChyZS50ZXN0KGRpc3BsYXlWYWx1ZShpdGVtVmFsdWUpKSkge1xuICAgICAgICByZXR1cm4gUHJvbWlzZS5yZXNvbHZlKHVuZGVmaW5lZCk7XG4gICAgICB9XG4gICAgICByZXR1cm4gUHJvbWlzZS5yZXNvbHZlKGBPYmplY3Qgb24gc3RyZWFtICR7aXRlbS5uYW1lfSBmYWlsZWQgcGF0dGVybiAke1xuICAgICAgICAgIGFnYWluc3RbJ2RhdGFQYXR0ZXJuJ119LiBGT1VORDogJHtkaXNwbGF5VmFsdWUoaXRlbVZhbHVlKX1gKTtcbiAgICB9IGVsc2UgaWYgKGl0ZW1WYWx1ZSA9PT0gYWdhaW5zdFZhbHVlKSB7XG4gICAgICByZXR1cm4gUHJvbWlzZS5yZXNvbHZlKHVuZGVmaW5lZCk7XG4gICAgfVxuICAgIHJldHVybiBQcm9taXNlLnJlc29sdmUoYE9iamVjdCBvbiBzdHJlYW0gJHtpdGVtLm5hbWV9IGZhaWxlZC4gRk9VTkQ6ICR7XG4gICAgICAgIGRpc3BsYXlWYWx1ZShpdGVtVmFsdWUpfSwgRVhQRUNURUQ6ICR7ZGlzcGxheVZhbHVlKGFnYWluc3RWYWx1ZSl9YCk7XG4gIH07XG4gIGVsZW1lbnQuY2xhc3NMaXN0LmFkZChpdGVtLm5hbWUpO1xuXG4gIGlmIChpdGVtLmRhdGFGaWxlKSB7XG4gICAgZWxlbWVudC50ZXh0Q29udGVudCA9IGF3YWl0IChhd2FpdCBmZXRjaChpdGVtLmRhdGFGaWxlKSkudGV4dCgpO1xuICB9IGVsc2UgaWYgKGl0ZW0uZGF0YVBhdHRlcm4pIHtcbiAgICBlbGVtZW50LnRleHRDb250ZW50ID0gSlNPTi5zdHJpbmdpZnkoe2RhdGE6IGl0ZW0uZGF0YX0pO1xuICB9IGVsc2UgaWYgKGl0ZW0uZGF0YSkge1xuICAgIGVsZW1lbnQudGV4dENvbnRlbnQgPSBKU09OLnN0cmluZ2lmeSh7ZGF0YTogaXRlbS5kYXRhfSk7XG4gIH1cblxuICBhcHBlbmRUby5hcHBlbmRDaGlsZChlbGVtZW50KTtcbiAgcmV0dXJuIFByb21pc2UucmVzb2x2ZSgpO1xufVxuXG4vKipcbiAqIENyZWF0ZXMgYWxsIG9mIHRoZSB0ZXN0IGl0ZW0gZW50cmllcyBmb3IgZWFjaCBpdGVtIGVudHJ5IHByb3ZpZGVkLlxuICovXG5hc3luYyBmdW5jdGlvbiBjcmVhdGVXb3Jrc3BhY2VJdGVtcyhcbiAgICBpdGVtczogVGVzdEl0ZW1FbnRyeVtdLCBhcHBlbmRUbzogSFRNTERpdkVsZW1lbnQpOiBQcm9taXNlPHZvaWQ+IHtcbiAgZm9yIChjb25zdCBpdGVtIG9mIGl0ZW1zKSB7XG4gICAgaWYgKGl0ZW0udHlwZSA9PT0gJ3ZpZGVvJykge1xuICAgICAgYXdhaXQgY3JlYXRlSW1hZ2VJdGVtKGl0ZW0sIGFwcGVuZFRvKTtcbiAgICB9IGVsc2Uge1xuICAgICAgYXdhaXQgY3JlYXRlT2JqZWN0SXRlbShpdGVtLCBhcHBlbmRUbyk7XG4gICAgfVxuICB9XG59XG5cbi8qKlxuICogQ29ubmVjdHMgYSB0ZXN0IGhhcm5lc3MgdXAgdG8gYSBzcGVjaWZpZWQgcm9vdCBub2RlICh1c3VhbGx5IGRvY3VtZW50LmJvZHkpLlxuICogQ2FsbCBgaW5pdGlhbGl6ZWAgdG8gc2V0IHVwIHRoZSBET00uXG4gKlxuICogVXNhZ2U6XG4gKiAxLiBQbGFjZSB0aGUgSlNPTi5zdHJpbmdpZnkgb2YgYSB0ZXN0IGNvbmZpZ3VyYXRpb24gaW50byBhbiBpbnB1dCBlbGVtZW50XG4gKiAgICB3aXRoIHRoZSBjbGFzcyAndGVzdCcgYW5kIHRoZW4gY2xpY2sgb24gYSBidXR0b24gd2l0aCB0aGUgY2xhc3MgJ3J1bi10ZXN0J1xuICogICAgYW5kIHdhaXQgZm9yIHRoZSBkaXYgdGFnIHdpdGggdGhlIGNsYXNzICcudGVzdC1jb21wbGV0ZScgdG8gc2hvdyB0aGUgdGV4dFxuICogICAgJ0NPTVBMRVRFJy5cbiAqL1xuZXhwb3J0IGNsYXNzIFRlc3RIYXJuZXNzIHtcbiAgcHJpdmF0ZSB0ZXN0RWxlbWVudCE6IEhUTUxUZXh0QXJlYUVsZW1lbnQ7XG4gIHByaXZhdGUgZXJyb3JzRWxlbWVudCE6IEhUTUxUZXh0QXJlYUVsZW1lbnQ7XG4gIHByaXZhdGUgcnVuVGVzdEJ1dHRvbiE6IEhUTUxCdXR0b25FbGVtZW50O1xuICBwcml2YXRlIHRlc3RDb21wbGV0ZUVsZW1lbnQhOiBIVE1MRGl2RWxlbWVudDtcbiAgcHJpdmF0ZSB0ZXN0UmVhZHlFbGVtZW50ITogSFRNTERpdkVsZW1lbnQ7XG4gIHByaXZhdGUgaW5wdXRzITogSFRNTERpdkVsZW1lbnQ7XG4gIHByaXZhdGUgZXhwZWN0cyE6IEhUTUxEaXZFbGVtZW50O1xuICBwcml2YXRlIG91dHB1dHMhOiBIVE1MRGl2RWxlbWVudDtcblxuICBwcml2YXRlIGFzeW5jIG9uUmVzdWx0KHJlc3VsdHM6IHNvbHV0aW9uc0FwaS5SZXN1bHRNYXAsIGNvbmZpZzogVGVzdENvbmZpZyk6XG4gICAgICBQcm9taXNlPHZvaWQ+IHtcbiAgICBjb25zdCBlcnJvcnM6IHN0cmluZ1tdID0gW107XG4gICAgZm9yIChjb25zdCBvdXRwdXQgb2YgY29uZmlnWyd0ZXN0J10hWydvdXRwdXRzJ10pIHtcbiAgICAgIGNvbnN0IGl0ZW1OYW1lID0gb3V0cHV0WyduYW1lJ107XG4gICAgICBhd2FpdCBvdXRwdXQucmVuZGVyIShyZXN1bHRzW2l0ZW1OYW1lXSk7XG4gICAgICBmb3IgKGNvbnN0IGV4cGVjdCBvZiBjb25maWdbJ3Rlc3QnXSFbJ2V4cGVjdHMnXSkge1xuICAgICAgICBpZiAoZXhwZWN0WyduYW1lJ10gPT09IGl0ZW1OYW1lKSB7XG4gICAgICAgICAgY29uc3QgZXJyb3JNc2cgPSBhd2FpdCBvdXRwdXQudmFsaWRhdGUhKGV4cGVjdCk7XG4gICAgICAgICAgaWYgKGVycm9yTXNnKSB7XG4gICAgICAgICAgICBlcnJvcnMucHVzaChlcnJvck1zZyk7XG4gICAgICAgICAgfVxuICAgICAgICAgIGJyZWFrO1xuICAgICAgICB9XG4gICAgICB9XG4gICAgfVxuICAgIHRoaXMuZXJyb3JzRWxlbWVudC50ZXh0Q29udGVudCA9IGVycm9ycy5qb2luKCdcXG4nKTtcbiAgICB0aGlzLnRlc3RDb21wbGV0ZUVsZW1lbnQudGV4dENvbnRlbnQgPSBDT01QTEVURTtcbiAgfVxuXG4gIC8qKlxuICAgKiBFeGVjdXRlcyB0aGUgdGVzdCBiYXNlZCBvbiB0aGUganNvbiBpbnN0cnVjdGlvbnMgc2l0dGluZyBpbiB0ZXN0RWxlbWVudC5cbiAgICogU2VlIGBvblJlc3VsdGAgZm9yIGhvdyB3ZSBsZWF2ZSBkYXRhIGZvciB0aGUgdGVzdCBoYXJuZXNzIHRvIHJlYWQuXG4gICAqL1xuICBhc3luYyBydW4oY29uZmlnOiBUZXN0Q29uZmlnKTogUHJvbWlzZTx2b2lkPiB7XG4gICAgaWYgKGNvbmZpZ1snc29sdXRpb25VcmwnXSkge1xuICAgICAgY29uc3Qgc29sdXRpb25Kc29uID0gYXdhaXQgKGF3YWl0IGZldGNoKGNvbmZpZ1snc29sdXRpb25VcmwnXSkpLmpzb24oKTtcbiAgICAgIGNvbmZpZ1snc29sdXRpb24nXSA9IHNvbHV0aW9uSnNvbiBhcyBzb2x1dGlvbnNBcGkuU29sdXRpb25Db25maWc7XG4gICAgfVxuICAgIGlmIChjb25maWdbJ3Rlc3RVcmwnXSkge1xuICAgICAgY29uc3QgdGVzdEpzb24gPSBhd2FpdCAoYXdhaXQgZmV0Y2goY29uZmlnWyd0ZXN0VXJsJ10pKS5qc29uKCk7XG4gICAgICBjb25maWdbJ3Rlc3QnXSA9IHRlc3RKc29uIGFzIFRlc3RDYXNlO1xuICAgIH1cbiAgICBpZiAoY29uZmlnWydiaW5hcnlwYiddKSB7XG4gICAgICBjb25maWdbJ3NvbHV0aW9uJ10hWydncmFwaCddID0geyd1cmwnOiBjb25maWdbJ2JpbmFyeXBiJ119O1xuICAgIH1cbiAgICBjb25zdCB0ZXN0TmFtZSA9IGNvbmZpZ1snbmFtZSddO1xuICAgIGNvbmZpZ1snc29sdXRpb24nXSFbJ2ZpbGVzJ10gPSBjb25maWdbJ3NvbHV0aW9uJ10hWydmaWxlcyddIHx8IFtdO1xuICAgIGNvbmZpZ1snc29sdXRpb24nXSFbJ2ZpbGVzJ10gPSBbXG4gICAgICAuLi5jb25maWdbJ3NvbHV0aW9uJ10hWydmaWxlcyddLFxuICAgICAge3VybDogYCR7dGVzdE5hbWV9X3dlYl9zb2x1dGlvbl9wYWNrZWRfYXNzZXRzX2xvYWRlci5qc2B9LFxuICAgICAge3NpbWQ6IGZhbHNlLCB1cmw6IGAke3Rlc3ROYW1lfV93ZWJfc29sdXRpb25fd2FzbV9iaW4uanNgfSxcbiAgICAgIHtzaW1kOiB0cnVlLCB1cmw6IGAke3Rlc3ROYW1lfV93ZWJfc29sdXRpb25fc2ltZF93YXNtX2Jpbi5qc2B9XG4gICAgXTtcblxuICAgIGNvbnN0IHNvbHV0aW9uID0gYXdhaXQgbmV3IFRlc3RTb2x1dGlvbihjb25maWdbJ3NvbHV0aW9uJ10hKS5pbml0aWFsaXplKCk7XG5cbiAgICBhd2FpdCBjcmVhdGVXb3Jrc3BhY2VJdGVtcyhjb25maWdbJ3Rlc3QnXSFbJ2lucHV0cyddLCB0aGlzLmlucHV0cyk7XG4gICAgYXdhaXQgY3JlYXRlV29ya3NwYWNlSXRlbXMoY29uZmlnWyd0ZXN0J10hWydleHBlY3RzJ10sIHRoaXMuZXhwZWN0cyk7XG4gICAgYXdhaXQgY3JlYXRlV29ya3NwYWNlSXRlbXMoY29uZmlnWyd0ZXN0J10hWydvdXRwdXRzJ10sIHRoaXMub3V0cHV0cyk7XG4gICAgYXdhaXQgZmx1c2hEb20oKTtcblxuICAgIGNvbnN0IGlucHV0czoge1trZXk6IHN0cmluZ106IHVua25vd259ID0ge307XG4gICAgZm9yIChjb25zdCBpbnB1dCBvZiBjb25maWdbJ3Rlc3QnXSFbJ2lucHV0cyddKSB7XG4gICAgICBpbnB1dHNbaW5wdXQubmFtZV0gPSBpbnB1dC5nZXRWYWx1ZSEoKTtcbiAgICB9XG4gICAgdGhpcy50ZXN0UmVhZHlFbGVtZW50LnRleHRDb250ZW50ID0gQ09NUExFVEU7XG4gICAgaWYgKGNvbmZpZ1sndGVzdCddIVsnb3B0aW9ucyddICYmIGNvbmZpZ1sndGVzdCddIVsnb3B0aW9ucyddWyd3YXJtdXAnXSkge1xuICAgICAgY29uc3Qgd2FybXVwID0gY29uZmlnWyd0ZXN0J10hWydvcHRpb25zJ11bJ3dhcm11cCddO1xuICAgICAgZm9yIChsZXQgbG9vcCA9IDA7IGxvb3AgPCB3YXJtdXA7IGxvb3ArKykge1xuICAgICAgICBhd2FpdCBzb2x1dGlvbi5zZW5kKGlucHV0cyk7XG4gICAgICB9XG4gICAgfVxuICAgIHNvbHV0aW9uLm9uUmVzdWx0cygoZGF0YSkgPT4ge1xuICAgICAgdGhpcy5vblJlc3VsdChkYXRhLCBjb25maWcpO1xuICAgIH0pO1xuICAgIGF3YWl0IHNvbHV0aW9uLnNlbmQoaW5wdXRzKTtcbiAgfVxuXG4gIC8qKlxuICAgKiBTZXRzIHVwIHRoZSB0ZXN0IGFuZCBzaG91bGQgYmUgY2FsbGVkIGJlZm9yZSBydW4gaXMgY2FsbGVkLiBCeSBkZXNpZ24sXG4gICAqIHJ1biB3aWxsIGJlIGNhbGxlZCBieSBzaW11bGF0aW5nIGEgY2xpY2sgb24gYSBidXR0b24gd2l0aCBjbGFzcyBgcnVuLXRlc3RgLlxuICAgKi9cbiAgYXN5bmMgaW5pdGlhbGl6ZSgpIHtcbiAgICB0aGlzLnRlc3RFbGVtZW50ID0gZG9jdW1lbnQucXVlcnlTZWxlY3RvcignLnRlc3QnKSBhcyBIVE1MVGV4dEFyZWFFbGVtZW50O1xuICAgIHRoaXMucnVuVGVzdEJ1dHRvbiA9XG4gICAgICAgIGRvY3VtZW50LnF1ZXJ5U2VsZWN0b3IoJy5ydW4tdGVzdCcpIGFzIEhUTUxCdXR0b25FbGVtZW50O1xuICAgIHRoaXMudGVzdENvbXBsZXRlRWxlbWVudCA9XG4gICAgICAgIGRvY3VtZW50LnF1ZXJ5U2VsZWN0b3IoJy50ZXN0LWNvbXBsZXRlJykgYXMgSFRNTERpdkVsZW1lbnQ7XG4gICAgdGhpcy5lcnJvcnNFbGVtZW50ID1cbiAgICAgICAgZG9jdW1lbnQucXVlcnlTZWxlY3RvcignLmVycm9ycycpIGFzIEhUTUxUZXh0QXJlYUVsZW1lbnQ7XG4gICAgdGhpcy50ZXN0UmVhZHlFbGVtZW50ID1cbiAgICAgICAgZG9jdW1lbnQucXVlcnlTZWxlY3RvcignLnRlc3QtcmVhZHknKSBhcyBIVE1MRGl2RWxlbWVudDtcbiAgICB0aGlzLm91dHB1dHMgPSBkb2N1bWVudC5xdWVyeVNlbGVjdG9yKCcub3V0cHV0cycpIGFzIEhUTUxEaXZFbGVtZW50O1xuICAgIHRoaXMuZXhwZWN0cyA9IGRvY3VtZW50LnF1ZXJ5U2VsZWN0b3IoJy5leHBlY3RzJykgYXMgSFRNTERpdkVsZW1lbnQ7XG4gICAgdGhpcy5pbnB1dHMgPSBkb2N1bWVudC5xdWVyeVNlbGVjdG9yKCcuaW5wdXRzJykgYXMgSFRNTERpdkVsZW1lbnQ7XG4gICAgdGhpcy5ydW5UZXN0QnV0dG9uLm9uY2xpY2sgPSAoKSA9PiB7XG4gICAgICB0aGlzLnJ1bihKU09OLnBhcnNlKHRoaXMudGVzdEVsZW1lbnQudmFsdWUpIGFzIFRlc3RDb25maWcpO1xuICAgIH07XG4gICAgYXdhaXQgZmx1c2hEb20oKTtcbiAgfVxufVxuIl19
;return exports;});

//third_party/mediapipe/web/solutions/test_harness/test_harness_driver.closure.js
goog.loadModule(function(exports) {'use strict';/**
 *
 * @fileoverview Load and initialize the test_harness. Interact with the page
 * to run the test.
 *
 * Generated from: third_party/mediapipe/web/solutions/test_harness/test_harness_driver.ts
 * @suppress {checkTypes,extraRequire,missingOverride,missingRequire,missingReturn,unusedPrivateMembers,uselessCode} checked by tsc
 */
goog.module('google3.third_party.mediapipe.web.solutions.test_harness.test_harness_driver');
var module = module || { id: 'third_party/mediapipe/web/solutions/test_harness/test_harness_driver.closure.js' };
goog.require('google3.third_party.javascript.tslib.tslib');
const tsickle_test_harness_1 = goog.requireType("google3.third_party.mediapipe.web.solutions.test_harness.test_harness");
const test_harness_1 = goog.require('google3.third_party.mediapipe.web.solutions.test_harness.test_harness');
/** @type {!tsickle_test_harness_1.TestHarness} */
const harness = new test_harness_1.TestHarness();
harness.initialize();
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoidGVzdF9oYXJuZXNzX2RyaXZlci5qcyIsInNvdXJjZVJvb3QiOiIiLCJzb3VyY2VzIjpbIi4uLy4uLy4uLy4uLy4uLy4uLy4uLy4uL3RoaXJkX3BhcnR5L21lZGlhcGlwZS93ZWIvc29sdXRpb25zL3Rlc3RfaGFybmVzcy90ZXN0X2hhcm5lc3NfZHJpdmVyLnRzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiI7Ozs7Ozs7Ozs7OztBQUtBLDZHQUFrRzs7TUFFNUYsT0FBTyxHQUFHLElBQUksMEJBQVcsRUFBRTtBQUNqQyxPQUFPLENBQUMsVUFBVSxFQUFFLENBQUMiLCJzb3VyY2VzQ29udGVudCI6WyIvKipcbiAqIEBmaWxlb3ZlcnZpZXcgTG9hZCBhbmQgaW5pdGlhbGl6ZSB0aGUgdGVzdF9oYXJuZXNzLiBJbnRlcmFjdCB3aXRoIHRoZSBwYWdlXG4gKiB0byBydW4gdGhlIHRlc3QuXG4gKi9cblxuaW1wb3J0IHtUZXN0SGFybmVzc30gZnJvbSAnZ29vZ2xlMy90aGlyZF9wYXJ0eS9tZWRpYXBpcGUvd2ViL3NvbHV0aW9ucy90ZXN0X2hhcm5lc3MvdGVzdF9oYXJuZXNzJztcblxuY29uc3QgaGFybmVzcyA9IG5ldyBUZXN0SGFybmVzcygpO1xuaGFybmVzcy5pbml0aWFsaXplKCk7XG4iXX0=
;return exports;});

