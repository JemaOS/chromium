
  var Module = typeof createMediapipeSolutionsPackedAssets !== 'undefined' ? createMediapipeSolutionsPackedAssets : {};
  
  if (!Module.expectedDataFileDownloads) {
    Module.expectedDataFileDownloads = 0;
  }
  Module.expectedDataFileDownloads++;
  (function() {
   var loadPackage = function(metadata) {
  
      var PACKAGE_PATH;
      if (typeof window === 'object') {
        PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.toString().substring(0, window.location.pathname.toString().lastIndexOf('/')) + '/');
      } else if (typeof location !== 'undefined') {
        // worker
        PACKAGE_PATH = encodeURIComponent(location.pathname.toString().substring(0, location.pathname.toString().lastIndexOf('/')) + '/');
      } else {
        throw 'using preloaded data can only be done on a web page or in a web worker';
      }
      var PACKAGE_NAME = 'blaze-out/k8-opt/genfiles/media/effectspipe/effects/effect/stylization/style_black_and_white_test_web_solution_packed_assets.data';
      var REMOTE_PACKAGE_BASE = 'style_black_and_white_test_web_solution_packed_assets.data';
      if (typeof Module['locateFilePackage'] === 'function' && !Module['locateFile']) {
        Module['locateFile'] = Module['locateFilePackage'];
        err('warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)');
      }
      var REMOTE_PACKAGE_NAME = Module['locateFile'] ? Module['locateFile'](REMOTE_PACKAGE_BASE, '') : REMOTE_PACKAGE_BASE;
    
      var REMOTE_PACKAGE_SIZE = metadata['remote_package_size'];
      var PACKAGE_UUID = metadata['package_uuid'];
    
      function fetchRemotePackage(packageName, packageSize, callback, errback) {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', packageName, true);
        xhr.responseType = 'arraybuffer';
        xhr.onprogress = function(event) {
          var url = packageName;
          var size = packageSize;
          if (event.total) size = event.total;
          if (event.loaded) {
            if (!xhr.addedTotal) {
              xhr.addedTotal = true;
              if (!Module.dataFileDownloads) Module.dataFileDownloads = {};
              Module.dataFileDownloads[url] = {
                loaded: event.loaded,
                total: size
              };
            } else {
              Module.dataFileDownloads[url].loaded = event.loaded;
            }
            var total = 0;
            var loaded = 0;
            var num = 0;
            for (var download in Module.dataFileDownloads) {
            var data = Module.dataFileDownloads[download];
              total += data.total;
              loaded += data.loaded;
              num++;
            }
            total = Math.ceil(total * Module.expectedDataFileDownloads/num);
            if (Module['setStatus']) Module['setStatus']('Downloading data... (' + loaded + '/' + total + ')');
          } else if (!Module.dataFileDownloads) {
            if (Module['setStatus']) Module['setStatus']('Downloading data...');
          }
        };
        xhr.onerror = function(event) {
          throw new Error("NetworkError for: " + packageName);
        }
        xhr.onload = function(event) {
          if (xhr.status == 200 || xhr.status == 304 || xhr.status == 206 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
            var packageData = xhr.response;
            callback(packageData);
          } else {
            throw new Error(xhr.statusText + " : " + xhr.responseURL);
          }
        };
        xhr.send(null);
      };

      function handleError(error) {
        console.error('package error:', error);
      };
    
        var fetchedCallback = null;
        var fetched = Module['getPreloadedPackage'] ? Module['getPreloadedPackage'](REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE) : null;

        if (!fetched) fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE, function(data) {
          if (fetchedCallback) {
            fetchedCallback(data);
            fetchedCallback = null;
          } else {
            fetched = data;
          }
        }, handleError);
      
    function runWithFS() {
  
      function assert(check, msg) {
        if (!check) throw msg + new Error().stack;
      }
  
          /** @constructor */
          function DataRequest(start, end, audio) {
            this.start = start;
            this.end = end;
            this.audio = audio;
          }
          DataRequest.prototype = {
            requests: {},
            open: function(mode, name) {
              this.name = name;
              this.requests[name] = this;
              Module['addRunDependency']('fp ' + this.name);
            },
            send: function() {},
            onload: function() {
              var byteArray = this.byteArray.subarray(this.start, this.end);
              this.finish(byteArray);
            },
            finish: function(byteArray) {
              var that = this;
      
          Module['FS_createPreloadedFile'](this.name, null, byteArray, true, true, function() {
            Module['removeRunDependency']('fp ' + that.name);
          }, function() {
            if (that.audio) {
              Module['removeRunDependency']('fp ' + that.name); // workaround for chromium bug 124926 (still no audio with this, but at least we don't hang)
            } else {
              err('Preloading file ' + that.name + ' failed');
            }
          }, false, true); // canOwn this data in the filesystem, it is a slide into the heap that will never change
  
              this.requests[this.name] = null;
            }
          };
      
              var files = metadata['files'];
              for (var i = 0; i < files.length; ++i) {
                new DataRequest(files[i]['start'], files[i]['end'], files[i]['audio']).open('GET', files[i]['filename']);
              }
      
        
      function processPackageData(arrayBuffer) {
        assert(arrayBuffer, 'Loading data file failed.');
        assert(arrayBuffer instanceof ArrayBuffer, 'bad input to processPackageData');
        var byteArray = new Uint8Array(arrayBuffer);
        var curr;
        
          // Reuse the bytearray from the XHR as the source for file reads.
          DataRequest.prototype.byteArray = byteArray;
    
            var files = metadata['files'];
            for (var i = 0; i < files.length; ++i) {
              DataRequest.prototype.requests[files[i].filename].onload();
            }
                Module['removeRunDependency']('datafile_blaze-out/k8-opt/genfiles/media/effectspipe/effects/effect/stylization/style_black_and_white_test_web_solution_packed_assets.data');

      };
      Module['addRunDependency']('datafile_blaze-out/k8-opt/genfiles/media/effectspipe/effects/effect/stylization/style_black_and_white_test_web_solution_packed_assets.data');
    
      if (!Module.preloadResults) Module.preloadResults = {};
    
        Module.preloadResults[PACKAGE_NAME] = {fromCache: false};
        if (fetched) {
          processPackageData(fetched);
          fetched = null;
        } else {
          fetchedCallback = processPackageData;
        }
      
    }
    if (Module['calledRun']) {
      runWithFS();
    } else {
      if (!Module['preRun']) Module['preRun'] = [];
      Module["preRun"].push(runWithFS); // FS is not initialized yet, wait for it
    }
  
   }
   loadPackage({"files": [{"filename": "/triggers_xref_2019_05_13_v0.xnxr", "start": 0, "end": 592, "audio": 0}, {"filename": "/segmentation-lite.f16.tflite", "start": 592, "end": 457152, "audio": 0}, {"filename": "/segmentation-full.f16.tflite", "start": 457152, "end": 765824, "audio": 0}, {"filename": "/overlay_STUDIO_TINT.png", "start": 765824, "end": 1075380, "audio": 0}, {"filename": "/overlay_LIGHT_LEAK.png", "start": 1075380, "end": 1221020, "audio": 0}, {"filename": "/overlay_GOOD_MORNING.png", "start": 1221020, "end": 1522963, "audio": 0}, {"filename": "/lut_VIVID.png", "start": 1522963, "end": 1524328, "audio": 0}, {"filename": "/lut_STUDIO_TINT.png", "start": 1524328, "end": 1527371, "audio": 0}, {"filename": "/lut_NOIR.png", "start": 1527371, "end": 1529012, "audio": 0}, {"filename": "/lut_LIGHT_LEAK.png", "start": 1529012, "end": 1533284, "audio": 0}, {"filename": "/lut_GOOD_MORNING.png", "start": 1533284, "end": 1539493, "audio": 0}, {"filename": "/lut_GOLD_2.png", "start": 1539493, "end": 1544436, "audio": 0}, {"filename": "/lut_GLOW_3.png", "start": 1544436, "end": 1548465, "audio": 0}, {"filename": "/lut_CHROMA_2.png", "start": 1548465, "end": 1552186, "audio": 0}, {"filename": "/lut_BLOCKBUSTER_fg.png", "start": 1552186, "end": 1552373, "audio": 0}, {"filename": "/lut_BLOCKBUSTER_bg.png", "start": 1552373, "end": 1555481, "audio": 0}, {"filename": "/lut_AUTO.png", "start": 1555481, "end": 1557720, "audio": 0}, {"filename": "/facerigs-full_roi_as_sub_head-2020_01_05-v0.f16.tflite", "start": 1557720, "end": 2789744, "audio": 0}, {"filename": "/facemesh_iris_faceflag-full_as_sub_head-2020_01_31-v0.f16.tflite", "start": 2789744, "end": 8489616, "audio": 0}, {"filename": "/facemesh-ultralite.f16.tflite", "start": 8489616, "end": 9228440, "audio": 0}, {"filename": "/facedetector-front.f16.tflite", "start": 9228440, "end": 9431712, "audio": 0}, {"filename": "/face_model_468_legacy.xnft", "start": 9431712, "end": 9460404, "audio": 0}, {"filename": "/face_model_468.xnft", "start": 9460404, "end": 9487408, "audio": 0}, {"filename": "/blend_shape_xref_duomoji.xnxr", "start": 9487408, "end": 9489744, "audio": 0}, {"filename": "/bg_STUDIO_TINT.png", "start": 9489744, "end": 9681496, "audio": 0}], "remote_package_size": 9681496, "package_uuid": "352ecf09-8003-450a-b4bc-4da6f405db75"});
  
  })();
  