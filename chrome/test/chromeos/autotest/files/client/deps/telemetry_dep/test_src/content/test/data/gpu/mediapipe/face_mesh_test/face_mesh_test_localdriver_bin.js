(function(){/*

 Copyright The Closure Library Authors.
 SPDX-License-Identifier: Apache-2.0
*/
'use strict';var d=this||self;function f(b,e){b=b.split(".");var a=d;b[0]in a||"undefined"==typeof a.execScript||a.execScript("var "+b[0]);for(var c;b.length&&(c=b.shift());)b.length||void 0===e?a[c]&&a[c]!==Object.prototype[c]?a=a[c]:a=a[c]={}:a[c]=e};document.querySelector(".test").value=JSON.stringify({binarypb:"face_mesh.binarypb",solutionUrl:"face_mesh_graph.json",testUrl:"face_mesh_test.json",name:"face_mesh_test"});f("runTest",function(){document.querySelector(".run-test").click()});f("isTestComplete",function(){return"COMPLETE"===document.querySelector(".test-complete").textContent});f("getErrors",function(){return document.querySelector(".errors").textContent});}).call(this);
