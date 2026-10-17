// Surface module/CDN failures instead of leaving the controls silently disabled.
window.addEventListener("error", (event) => {
  const error = document.getElementById("error");
  error.hidden = false;
  error.textContent =
    event.message ||
    "A script failed to load. Check your connection and reload.";
});
