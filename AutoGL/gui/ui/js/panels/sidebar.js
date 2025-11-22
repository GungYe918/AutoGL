// js/panels/sidebar.js
export class SidebarPanel {
    static toggleExplorer() {
        const sidebar      = document.getElementById("sidebar");
        const editorWrapper = document.getElementById("editor-wrapper");
        const btnExplorer  = document.getElementById("btn-explorer");

        const hidden = sidebar.classList.contains("hidden");

        // 클래스 토글: 레이아웃은 아래에서 transition 끝난 뒤에 처리
        if (hidden) {
            // 사이드바 열기
            sidebar.classList.remove("hidden");
            editorWrapper.classList.remove("sidebar-hidden");
            btnExplorer.classList.add("active");
        } else {
            // 사이드바 닫기
            sidebar.classList.add("hidden");
            editorWrapper.classList.add("sidebar-hidden");
            btnExplorer.classList.remove("active");
        }

        // transition 끝난 후에 monaco 레이아웃 맞추기
        const doLayout = () => {
            if (window.editorInstance) {
                window.editorInstance.layout();
            }
        };

        // CSS transition(left) 이 끝난 순간에 한번 레이아웃
        const onTransitionEnd = (ev) => {
            if (ev.propertyName === "left") {
                editorWrapper.removeEventListener("transitionend", onTransitionEnd);
                doLayout();
            }
        };
        editorWrapper.addEventListener("transitionend", onTransitionEnd);

        // 혹시 transition 이벤트를 못 받는 경우를 위한 안전망
        setTimeout(() => {
            editorWrapper.removeEventListener("transitionend", onTransitionEnd);
            doLayout();
        }, 350); // #editor-wrapper의 transition: left 0.25s 이라서 약간 여유 줌

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
