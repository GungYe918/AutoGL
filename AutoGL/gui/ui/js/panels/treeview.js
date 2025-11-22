// js/panels/treeview.js
export class TreeView {

    /** 파일 확장자 기반 Monaco 언어 감지 (확장 버전) */
    static detectLanguage(path) {
        const p = path.toLowerCase();

        // ---- C / C++ ----
        if (p.endsWith(".c")) return "c";
        if (p.endsWith(".h") || p.endsWith(".hpp") ||
            p.endsWith(".cpp") || p.endsWith(".cc") ||
            p.endsWith(".cxx") || p.endsWith(".hh") ||
            p.endsWith(".ino")) return "cpp";

        // ---- Rust ----
        if (p.endsWith(".rs")) return "rust";

        // ---- C# ----
        if (p.endsWith(".cs")) return "csharp";

        // ---- GLSL/HLSL/Shaders ----
        if (p.endsWith(".glsl") || p.endsWith(".vert") || p.endsWith(".frag") ||
            p.endsWith(".vs")   || p.endsWith(".fs")   || p.endsWith(".geom") ||
            p.endsWith(".comp") || p.endsWith(".tesc") || p.endsWith(".tese")) {
            return "glsl";
        }
        if (p.endsWith(".hlsl")) return "hlsl";

        // ---- Scripting languages ----
        if (p.endsWith(".js") || p.endsWith(".mjs")) return "javascript";
        if (p.endsWith(".ts")) return "typescript";
        if (p.endsWith(".lua")) return "lua";
        if (p.endsWith(".py")) return "python";
        if (p.endsWith(".rb")) return "ruby";
        if (p.endsWith(".php")) return "php";
        if (p.endsWith(".d")) return "d";

        // ---- Web ----
        if (p.endsWith(".html") || p.endsWith(".htm")) return "html";
        if (p.endsWith(".css") || p.endsWith(".scss") || p.endsWith(".sass")) return "css";
        if (p.endsWith(".xml") || p.endsWith(".svg")) return "xml";

        // ---- Build / Config / Data ----
        if (p.endsWith(".json")) return "json";
        if (p.endsWith(".toml")) return "toml";
        if (p.endsWith(".yaml") || p.endsWith(".yml")) return "yaml";
        if (p.endsWith(".cmake") || p.endsWith("cMakeLists.txt".toLowerCase()))
            return "cmake";

        // ---- Markdown / Text ----
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
        const lang = TreeView.detectLanguage(data.path || "");

        const model = editor.getModel();
        if (!model) return;

        // --- 언어 변경: Monaco의 Lint는 여기서 자동 활성화됨 ---
        try {
            monaco.editor.setModelLanguage(model, lang);
        } catch (e) {
            console.error("[TreeView] setModelLanguage failed:", e);
        }

        // --- 모델 내용 업데이트 ---
        editor.setValue(data.content || "");

        // --- 언어 변경 후 Lint 강제 트리거 ---
        if (monaco.languages && monaco.languages.getLanguages) {
            try {
                // 일부 언어는 진단 제공
                if (lang === "json" || lang === "yaml" ||
                    lang === "css"  || lang === "html" ||
                    lang === "javascript" || lang === "typescript") {
                    monaco.editor.setModelMarkers(model, lang, []);
                }
            } catch (e) {
                console.warn("[TreeView] lint trigger failed:", e);
            }
        }
        
        window.__currentOpenedFile = data.path;
    }






    /** JS에서 최초로 C++에게 트리 요청 */
    static requestRootTree() {
        const req = JSON.stringify({ action: "list_root" });
        sendToPanel("treeview", req);
    }

    /** C++에서 받은 JSON 트리를 렌더링 */
    static render(treeData) {
        const container = document.getElementById("file-tree");
        container.innerHTML = TreeView.renderNode(treeData, "");
        TreeView.attachEvents();
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
                // 폴더 노드
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
                // 파일 노드
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
        // 폴더 헤더(.tree-folder-node)는 무시하고, 진짜 파일 노드만 선택
        const fileNodes = document.querySelectorAll(".tree-node.tree-file");

        fileNodes.forEach(node => {
            if (node.dataset.fileBound === "true") return;
            node.dataset.fileBound = "true";

            node.addEventListener("click", e => {
                e.stopPropagation();
                const path = node.dataset.path;
                // console.log("[TreeView] file click:", path);
                TreeView.requestFile(path);
            });
        });
    }

    static attachEvents() {
        const folderHeaderNodes = document.querySelectorAll(".tree-folder-node");

        folderHeaderNodes.forEach(node => {
            if (node.dataset.bound === "true") return;
            node.dataset.bound = "true";

            node.addEventListener("click", e => {
                e.stopPropagation();

                const folder = node.parentElement; // .tree-folder
                const arrow  = node.querySelector(".tree-arrow");
                const folderPath = node.dataset.path;
                const childrenContainer = folder.querySelector(".tree-children");

                if (folder.classList.contains("open")) {
                    folder.classList.remove("open");
                    arrow.textContent = "chevron_right";
                } else {
                    folder.classList.add("open");
                    arrow.textContent = "expand_more";
                    TreeView.requestChild(folderPath, childrenContainer);
                }
            });
        });

        // 새로 추가된 파일 노드에도 항상 파일 핸들러 붙이기
        TreeView.attachFileEvents();
    }

    static requestChild(folderPath, targetNode) {
        const msg = JSON.stringify({
            action: "list_child",
            folder: folderPath
        });

        TreeView._pendingNode = targetNode;
        TreeView._pendingPath = folderPath;
        sendToPanel("treeview", msg);
    }

    static insertChildTree(json) {
        if (!TreeView._pendingNode) return;

        const parentPath = TreeView._pendingPath || "";
        TreeView._pendingNode.innerHTML =
            TreeView.renderNode(json, parentPath);

        // 새 노드들에 폴더/파일 이벤트 다시 바인딩
        TreeView.attachEvents();

        TreeView._pendingNode = null;
        TreeView._pendingPath = null;
    }
}
