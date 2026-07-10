/* Front-website behaviour. Currently: copy-to-clipboard buttons on code blocks.
 * Vanilla JS, no dependencies. The buttons themselves are emitted server-side by
 * docs/build_guides.py; this only wires up the click. */
(function () {
  "use strict";

  function copyText(text) {
    if (navigator.clipboard && navigator.clipboard.writeText) {
      return navigator.clipboard.writeText(text);
    }
    // Fallback for insecure contexts (e.g. opening the files over file://).
    return new Promise(function (resolve, reject) {
      var ta = document.createElement("textarea");
      ta.value = text;
      ta.setAttribute("readonly", "");
      ta.style.position = "fixed";
      ta.style.opacity = "0";
      document.body.appendChild(ta);
      ta.select();
      var ok = false;
      try { ok = document.execCommand("copy"); } catch (e) { ok = false; }
      document.body.removeChild(ta);
      ok ? resolve() : reject();
    });
  }

  function flash(btn, label, cls) {
    btn.textContent = label;
    if (cls) btn.classList.add(cls);
    clearTimeout(btn._t);
    btn._t = setTimeout(function () {
      btn.textContent = "Copy";
      btn.classList.remove("copied");
    }, 1600);
  }

  document.addEventListener("click", function (e) {
    var btn = e.target.closest(".copy");
    if (!btn) return;
    var pre = btn.closest("pre");
    var code = pre && pre.querySelector("code");
    if (!code) return;
    copyText(code.innerText).then(
      function () { flash(btn, "Copied", "copied"); },
      function () { flash(btn, "Press Ctrl+C"); }
    );
  });
})();
