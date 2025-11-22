// main.js static bootstrap

window.AutoGL_LoadedModules = window.AutoGL_LoadedModules || {};

console.log("[JS] main.js loaded");

// __AUTOGL_IMPORTS_START__
import * as mod_panels_sidebar from "./panels/sidebar.js";
window.AutoGL_LoadedModules["panels/sidebar.js"] = mod_panels_sidebar;

import * as mod_panels_treeview from "./panels/treeview.js";
window.AutoGL_LoadedModules["panels/treeview.js"] = mod_panels_treeview;

// __AUTOGL_IMPORTS_END__