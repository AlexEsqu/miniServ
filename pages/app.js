const tablistButtons = document.getElementsByClassName("tablist-button");
const presentationArticles = document.getElementsByClassName("presentation-article");

function updateArticleVisibility(clickedButton) {
	Array.from(presentationArticles).forEach((article) => {
		if (article.getAttribute("id") === clickedButton.getAttribute("aria-controls")) {
			article.removeAttribute("hidden");
		} else {
			article.setAttribute("hidden", "hidden");
		}
	});
}

function toggleTablistAriaSelection(clickedButton) {
	Array.from(tablistButtons).forEach((button) => {
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

crossButttons = document.getElementsByClassName("Close");

Array.from(crossButttons).forEach((button) => {
	button.addEventListener("click", (e) => {
		button.closest(".window").style.display = "none";
	});
});

Array.from(tablistButtons).forEach((button) => {
	button.addEventListener("click", (e) => tablistHandler(e));
});


const windows = document.getElementsByClassName("window");

let offsetX,
	offsetY,
	activeWindow = null,
	i = 1;

for (let w of windows) {
	w.addEventListener("mousedown", (e) => {
		activeWindow = w;

		// Bring the active window to front (optional)
		activeWindow.style.zIndex = 10 + i;
		Array.from(activeWindow.getElementsByTagName("*")).forEach((child) => {
			child.style.zIndex = 10 + i;
		});
		// Calculate offset between cursor and window top-left corner
		offsetX = e.clientX - activeWindow.getBoundingClientRect().left;
		offsetY = e.clientY - activeWindow.getBoundingClientRect().top;

		// Add global listeners
		document.addEventListener("mousemove", mouseMoveHandler);
		document.addEventListener("mouseup", mouseUpHandler);
		i++;
	});
}

function mouseMoveHandler(e) {
	if (!activeWindow) return;
	activeWindow.style.position = "absolute";
	activeWindow.style.left = `${e.clientX - offsetX}px`;
	activeWindow.style.top = `${e.clientY - offsetY}px`;
}

function mouseUpHandler() {
	if (!activeWindow) return;

	activeWindow = null;
	document.removeEventListener("mousemove", mouseMoveHandler);
	document.removeEventListener("mouseup", mouseUpHandler);
}
