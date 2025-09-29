const buttons = document.getElementsByClassName("tablist-button");
const presentationArticles = document.getElementsByClassName(
  "presentation-article"
);

function updateArticleVisibility(clickedButton) {
  Array.from(presentationArticles).forEach((article) => {
    if (
      article.getAttribute("id") === clickedButton.getAttribute("aria-controls")
    ) {
      article.removeAttribute("hidden");
    } else {
      article.setAttribute("hidden", "hidden");
    }
  });
}

function toggleTablistAriaSelection(clickedButton) {
  Array.from(buttons).forEach((button) => {
    if (button === clickedButton) {
      button.setAttribute("aria-selected", "true");
    } else button.setAttribute("aria-selected", "false");
  });
}

function tablistHandler(e) {
  clickedButton = e.target;
  toggleTablistAriaSelection(clickedButton);
  updateArticleVisibility(clickedButton);

  formButtons = document.getElementById("form-buttons");
  if (e.target.getAttribute("aria-controls") === "add-yourself-article") {
    formButtons.style.display = "inline";
  } else {
    formButtons.style.display = "none";
  }
}

Array.from(buttons).forEach((button) => {
  button.addEventListener("click", (e) => tablistHandler(e));
});
