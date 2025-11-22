// js/panels/sidebar.js
export class SidebarPanel {
    static toggleExplorer() {
        const sidebar = document.getElementById("sidebar");
        const editor  = document.getElementById("editor");
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

    static toggleExplorer() {
        const sidebar = document.getElementById("sidebar");
        const editorWrapper = document.getElementById("editor-wrapper");
        const editor = document.getElementById("editor");
        const btnExplorer = document.getElementById("btn-explorer");

        const hidden = sidebar.classList.contains("hidden");

        if (hidden) {
            sidebar.classList.remove("hidden");
            editorWrapper.classList.remove("sidebar-hidden");
            editor.classList.remove("sidebar-hidden");
            btnExplorer.classList.add("active");
        } else {
            sidebar.classList.add("hidden");
            editorWrapper.classList.add("sidebar-hidden");
            editor.classList.add("sidebar-hidden");
            btnExplorer.classList.remove("active");
        }

        if (window.editorInstance) {
            setTimeout(() => {
                window.editorInstance.layout();
            }, 10);
        }

        sendToPanel("sidebar", "open_explorer");
    }




    /** ToolBox toggle 버튼 컨트롤 */
    static initToolBox() {
        const toolboxBtn = document.getElementById("btn-toolbox");
        const panel = document.getElementById("toolbox-panel");

        if (!toolboxBtn || !panel) return;

        toolboxBtn.addEventListener("click", () => {
            panel.classList.toggle("open");
        });

        // 저장
        document.getElementById("btn-save").addEventListener("click", () => {
            SidebarPanel.saveFile();
        });

        // undo
        document.getElementById("btn-undo").addEventListener("click", () => {
            if (window.editorInstance) window.editorInstance.trigger("kb", "undo", null);
        });

        // redo
        document.getElementById("btn-redo").addEventListener("click", () => {
            if (window.editorInstance) window.editorInstance.trigger("kb", "redo", null);
        });
    }

    static saveFile() {
        if (!window.editorInstance || !window.__currentOpenedFile) return;

        const content = window.editorInstance.getValue();
        const msg = JSON.stringify({
            action: "save_file",
            file: window.__currentOpenedFile,
            content: content
        });

        sendToPanel("treeview", msg);
    }
}
