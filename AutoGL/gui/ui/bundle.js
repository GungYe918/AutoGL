(() => {
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
    /** Render the full tree */
    static render(treeData) {
      const container = document.getElementById("file-tree");
      container.innerHTML = _TreeView.renderNode(treeData);
      _TreeView.attachEvents();
    }
    /** Render a single node (folder or file) */
    static renderNode(obj) {
      let html = "";
      for (let name in obj) {
        const val = obj[name];
        if (typeof val === "object") {
          html += `
                <div class="tree-folder">
                    <div class="tree-node">
                        <span class="tree-arrow material-icons">chevron_right</span>
                        <span class="material-icons">folder</span>
                        <span>${name}</span>
                    </div>
                    <div class="tree-children">
                        ${_TreeView.renderNode(val)}
                    </div>
                </div>
                `;
        } else {
          html += `
                <div class="tree-node">
                    <span class="tree-arrow"></span>
                    <span class="material-icons">insert_drive_file</span>
                    <span>${name}</span>
                </div>
                `;
        }
      }
      return html;
    }
    /** Attach event listeners for folder toggle */
    static attachEvents() {
      const folders = document.querySelectorAll(".tree-folder > .tree-node");
      folders.forEach((node) => {
        node.addEventListener("click", (e) => {
          e.stopPropagation();
          const folder = node.parentElement;
          const arrow = node.querySelector(".tree-arrow");
          folder.classList.toggle("open");
          if (folder.classList.contains("open"))
            arrow.textContent = "expand_more";
          else
            arrow.textContent = "chevron_right";
        });
      });
    }
  };

  // js/main.tmp.js
  window.AutoGL_LoadedModules = window.AutoGL_LoadedModules || {};
  console.log("[JS] main.js loaded");
  window.AutoGL_LoadedModules["panels/sidebar.js"] = sidebar_exports;
  window.AutoGL_LoadedModules["panels/treeview.js"] = treeview_exports;
})();
//# sourceMappingURL=bundle.js.map
