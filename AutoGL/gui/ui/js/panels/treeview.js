// js/panels/treeview.js
export class TreeView {

    /** Render the full tree */
    static render(treeData) {
        const container = document.getElementById("file-tree");
        container.innerHTML = TreeView.renderNode(treeData);
        TreeView.attachEvents();
    }

    /** Render a single node (folder or file) */
    static renderNode(obj) {
        let html = "";

        for (let name in obj) {
            const val = obj[name];

            // Folder
            if (typeof val === "object") {
                html += `
                <div class="tree-folder">
                    <div class="tree-node">
                        <span class="tree-arrow material-icons">chevron_right</span>
                        <span class="material-icons">folder</span>
                        <span>${name}</span>
                    </div>
                    <div class="tree-children">
                        ${TreeView.renderNode(val)}
                    </div>
                </div>
                `;
            }
            // File
            else {
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

        folders.forEach(node => {
            node.addEventListener("click", e => {
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
}
