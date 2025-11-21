export class SidebarPanel {
    constructor() {}

    init() {
        document.getElementById("sidebar").innerHTML = `
            <div class="sidebar-icons">
                <span class="material-icons" onclick="SidebarPanel.openExplorer()">folder</span>
                <span class="material-icons">search</span>
                <span class="material-icons">settings</span>
            </div>
        `;
    }

    static openExplorer() {
        sendToPanel("sidebar", "open_explorer");
    }
}
