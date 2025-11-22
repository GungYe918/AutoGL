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
}
