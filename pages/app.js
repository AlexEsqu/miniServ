const buttons = document.getElementsByClassName("tablist-button");
const presentationArticles = document.getElementsByClassName("presentation-article");

function updateArticleVisibility() {
  Array.from(presentationArticles).forEach((article) => {
    if (article.getAttribute("hidden") == null) {
      article.setAttribute("hidden", "hidden");
    } else {
      article.removeAttribute("hidden");
    }
  });
}

function toggleTablistAriaSelection() {
  state = undefined;
  Array.from(buttons).forEach((button) => {
    state = button.getAttribute("aria-selected") === "true" ? "false" : "true"
    button.setAttribute("aria-selected", state);
  });
}

function tablistHandler() {
  toggleTablistAriaSelection();
  updateArticleVisibility();
}

// console.log(buttons);
Array.from(buttons).forEach((button) => {
  button.addEventListener("click", () => tablistHandler());
});
