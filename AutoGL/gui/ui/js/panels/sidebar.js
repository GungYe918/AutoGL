// js/panels/sidebar.js

export class SidebarPanel {
    constructor() {}

    init() {}

    static openExplorer() {
        sendToPanel("sidebar", "open_explorer");
    }

    /** Render full file tree from JSON */
    static renderFileTree(tree) {
        const container = document.getElementById("file-tree");
        container.innerHTML = SidebarPanel.renderNode(tree);
        SidebarPanel.attachToggleEvents();
    }

    static renderNode(obj) {
        let html = "";

        for (let key in obj) {
            const val = obj[key];

            // Folder
            if (typeof val === "object") {
                html += `
                    <div class="tree-folder tree-item">
                        <span class="material-icons">folder</span>
                        <span>${key}</span>
                        <div class="tree-folder-children">
                            ${SidebarPanel.renderNode(val)}
                        </div>
                    </div>
                `;
            }
            // File
            else {
                html += `
                    <div class="tree-item">
                        <span class="material-icons">insert_drive_file</span>
                        <span>${key}</span>
                    </div>
                `;
            }
        }

        return html;
    }

    /** Enable click-to-toggle for folder items */
    static attachToggleEvents() {
        const items = document.querySelectorAll(".tree-folder");

        items.forEach(item => {
            item.addEventListener("click", (e) => {
                e.stopPropagation();
                item.classList.toggle("open");
                const icon = item.querySelector(".material-icons");
                icon.textContent = item.classList.contains("open")
                    ? "folder_open"
                    : "folder";
            });
        });
    }
}
