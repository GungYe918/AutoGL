var AutoGLBundle = (() => {
  var __defProp = Object.defineProperty;
  var __export = (target, all) => {
    for (var name in all)
      __defProp(target, name, { get: all[name], enumerable: true });
  };

  // js/panels/sidebar.js
  var sidebar_exports = {};
  __export(sidebar_exports, {
    SidebarPanel: () => SidebarPanel
  });
  var SidebarPanel = class {
    static toggleExplorer() {
      const sidebar = document.getElementById("sidebar");
      const editor = document.getElementById("editor");
      const btnExplorer = document.getElementById("btn-explorer");
      const hidden = sidebar.classList.contains("hidden");
      if (hidden) {
        sidebar.classList.remove("hidden");
        editor.classList.remove("sidebar-hidden");
        btnExplorer.classList.add("active");
      } else {
        sidebar.classList.add("hidden");
        editor.classList.add("sidebar-hidden");
        btnExplorer.classList.remove("active");
      }
      sendToPanel("sidebar", "open_explorer");
    }
  };

  // js/panels/treeview.js
  var treeview_exports = {};
  __export(treeview_exports, {
    TreeView: () => TreeView
  });
  var TreeView = class _TreeView {
    /** 파일 확장자 기반 Monaco 언어 감지 (확장 버전) */
    static detectLanguage(path) {
      const p = path.toLowerCase();
      if (p.endsWith(".c")) return "c";
      if (p.endsWith(".h") || p.endsWith(".hpp") || p.endsWith(".cpp") || p.endsWith(".cc") || p.endsWith(".cxx") || p.endsWith(".hh") || p.endsWith(".ino")) return "cpp";
      if (p.endsWith(".rs")) return "rust";
      if (p.endsWith(".cs")) return "csharp";
      if (p.endsWith(".glsl") || p.endsWith(".vert") || p.endsWith(".frag") || p.endsWith(".vs") || p.endsWith(".fs") || p.endsWith(".geom") || p.endsWith(".comp") || p.endsWith(".tesc") || p.endsWith(".tese")) {
        return "glsl";
      }
      if (p.endsWith(".hlsl")) return "hlsl";
      if (p.endsWith(".js") || p.endsWith(".mjs")) return "javascript";
      if (p.endsWith(".ts")) return "typescript";
      if (p.endsWith(".lua")) return "lua";
      if (p.endsWith(".py")) return "python";
      if (p.endsWith(".rb")) return "ruby";
      if (p.endsWith(".php")) return "php";
      if (p.endsWith(".d")) return "d";
      if (p.endsWith(".html") || p.endsWith(".htm")) return "html";
      if (p.endsWith(".css") || p.endsWith(".scss") || p.endsWith(".sass")) return "css";
      if (p.endsWith(".xml") || p.endsWith(".svg")) return "xml";
      if (p.endsWith(".json")) return "json";
      if (p.endsWith(".toml")) return "toml";
      if (p.endsWith(".yaml") || p.endsWith(".yml")) return "yaml";
      if (p.endsWith(".cmake") || p.endsWith("cMakeLists.txt".toLowerCase()))
        return "cmake";
      if (p.endsWith(".md") || p.endsWith(".markdown")) return "markdown";
      if (p.endsWith(".txt")) return "plaintext";
      return "plaintext";
    }
    /** 파일 로딩 처리 (C++에서 메시지 도착 시 호출됨) */
    static handleFileLoad(data) {
      if (!window.__monacoReady || !window.editorInstance || typeof monaco === "undefined") {
        window.__pendingFileLoads.push(data);
        return;
      }
      const editor = window.editorInstance;
      const lang = _TreeView.detectLanguage(data.path || "");
      const model = editor.getModel();
      if (!model) return;
      try {
        monaco.editor.setModelLanguage(model, lang);
      } catch (e) {
        console.error("[TreeView] setModelLanguage failed:", e);
      }
      editor.setValue(data.content || "");
      if (monaco.languages && monaco.languages.getLanguages) {
        try {
          if (lang === "json" || lang === "yaml" || lang === "css" || lang === "html" || lang === "javascript" || lang === "typescript") {
            monaco.editor.setModelMarkers(model, lang, []);
          }
        } catch (e) {
          console.warn("[TreeView] lint trigger failed:", e);
        }
      }
    }
    /** JS에서 최초로 C++에게 트리 요청 */
    static requestRootTree() {
      const req = JSON.stringify({ action: "list_root" });
      sendToPanel("treeview", req);
    }
    /** C++에서 받은 JSON 트리를 렌더링 */
    static render(treeData) {
      const container = document.getElementById("file-tree");
      container.innerHTML = _TreeView.renderNode(treeData, "");
      _TreeView.attachEvents();
    }
    /** 특정 파일 클릭 시 C++에게 읽기 요청 */
    static requestFile(fileName) {
      const msg = JSON.stringify({
        action: "read_file",
        file: fileName
      });
      sendToPanel("treeview", msg);
    }
    static renderNode(obj, parentPath = "") {
      let html = "";
      for (let name in obj) {
        const value = obj[name];
        const fullPath = parentPath ? `${parentPath}/${name}` : name;
        if (typeof value === "object") {
          html += `
                <div class="tree-folder" data-path="${fullPath}">
                    <div class="tree-node tree-folder-node" data-path="${fullPath}">
                        <span class="tree-arrow material-icons">chevron_right</span>
                        <span class="material-icons">folder</span>
                        <span>${name}</span>
                    </div>
                    <div class="tree-children"></div>
                </div>`;
        } else {
          html += `
                <div class="tree-node tree-file" data-path="${fullPath}">
                    <span class="tree-arrow"></span>
                    <span class="material-icons">insert_drive_file</span>
                    <span>${name}</span>
                </div>`;
        }
      }
      return html;
    }
    static attachFileEvents() {
      const fileNodes = document.querySelectorAll(".tree-node.tree-file");
      fileNodes.forEach((node) => {
        if (node.dataset.fileBound === "true") return;
        node.dataset.fileBound = "true";
        node.addEventListener("click", (e) => {
          e.stopPropagation();
          const path = node.dataset.path;
          _TreeView.requestFile(path);
        });
      });
    }
    static attachEvents() {
      const folderHeaderNodes = document.querySelectorAll(".tree-folder-node");
      folderHeaderNodes.forEach((node) => {
        if (node.dataset.bound === "true") return;
        node.dataset.bound = "true";
        node.addEventListener("click", (e) => {
          e.stopPropagation();
          const folder = node.parentElement;
          const arrow = node.querySelector(".tree-arrow");
          const folderPath = node.dataset.path;
          const childrenContainer = folder.querySelector(".tree-children");
          if (folder.classList.contains("open")) {
            folder.classList.remove("open");
            arrow.textContent = "chevron_right";
          } else {
            folder.classList.add("open");
            arrow.textContent = "expand_more";
            _TreeView.requestChild(folderPath, childrenContainer);
          }
        });
      });
      _TreeView.attachFileEvents();
    }
    static requestChild(folderPath, targetNode) {
      const msg = JSON.stringify({
        action: "list_child",
        folder: folderPath
      });
      _TreeView._pendingNode = targetNode;
      _TreeView._pendingPath = folderPath;
      sendToPanel("treeview", msg);
    }
    static insertChildTree(json) {
      if (!_TreeView._pendingNode) return;
      const parentPath = _TreeView._pendingPath || "";
      _TreeView._pendingNode.innerHTML = _TreeView.renderNode(json, parentPath);
      _TreeView.attachEvents();
      _TreeView._pendingNode = null;
      _TreeView._pendingPath = null;
    }
  };

  // js/main.tmp.js
  window.AutoGL_LoadedModules = window.AutoGL_LoadedModules || {};
  console.log("[JS] main.js loaded");
  window.AutoGL_LoadedModules["panels/sidebar.js"] = sidebar_exports;
  window.AutoGL_LoadedModules["panels/treeview.js"] = treeview_exports;
})();
//# sourceMappingURL=bundle.js.map
