const tablistButtons = document.getElementsByClassName("tablist-button");
const presentationArticles = document.getElementsByClassName("presentation-article");
let formSubmitted = false;

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

const form = document.querySelector("form");

async function sendForm() {
	const formData = new FormData(form);
	try {
		const response = await fetch("/post", {
			method: "POST",
			body: formData,
		});

		if (response.ok) {
			console.log("Data:", response.body);
			// Display success message or perform any other action
		} else {
			console.error("Error:", response.status);
		}
	} catch (error) {
		console.error(error);
	}
}

async function getFormInfo(endpoint, responseType = "text") {
	return new Promise((resolve, reject) => {
		const req = new XMLHttpRequest();
		req.open("GET", "http://localhost:8080/post/" + endpoint);
		req.responseType = responseType;
		req.onload = () => {
			if (req.readyState == 4 && req.status >= 200 && req.status < 300) {
				resolve(req.response);
			} else {
				reject(new Error(`Error: ${req.status}`));
			}
		};
		req.onerror = () => {
			reject(new Error("Network error"));
		};
		req.send();
	});
}

async function getFormData() {
	let name, description, picture;
	const article = document.getElementById("add-yourself-article");
	try {
		name = await getFormInfo("name", "text");
		description = await getFormInfo("description", "text");
		picture = await getFormInfo("picture", "blob");
		imageUrl = URL.createObjectURL(picture);
		document.getElementById("tablist-button-add-yourself").textContent = name;
		if (localStorage.getItem("formSubmitted") == "true");
		{
			for (let i = 0; i < article.children.length; i++) {
				article.children[i].style.display = "none";
			}
			document.getElementById("form-button-ok").style.display = "none";
			document.getElementById("form-button-cancel").style.display = "none";
		if (picture) {
			const img = document.createElement("img");
			img.src = imageUrl;
			article.appendChild(img);
		}
		if (description) {
			const p = document.createElement("p");
			p.textContent = description;
			article.appendChild(p);
		}
		}

	} catch (error) {
		localStorage.setItem("formSubmitted", "false");
		console.error(error);
		console.log("form set to false");
		return;
	}
}

form.addEventListener("submit", async function (e) {
	e.preventDefault(); // Prevents the default form submission behavior
	localStorage.setItem("formSubmitted", "true");
	sendForm();
	getFormData();
});

window.addEventListener("load", async (event) => {
	if (localStorage.getItem("formSubmitted") == "true") {
		getFormData();
	}
});

// Delete Button
var deleteButton = document.getElementById("delete-button");

async function deleteFormInfo(endpoint, responseType = "text") {
	return new Promise((resolve, reject) => {
		const req = new XMLHttpRequest();
		req.open("DELETE", "http://localhost:8080/post/" + endpoint);
		req.responseType = responseType;
		req.onload = () => {
			if (req.readyState == 4 && req.status >= 200 && req.status < 300) {
				resolve(req.response);
			} else {
				reject(new Error(`Error: ${req.status}`));
			}
		};
		req.onerror = () => {
			reject(new Error("Network error"));
		};
		req.send();
	});
}

deleteButton.addEventListener("click", async (e) => {
	if (window.confirm("Are you sure you want to delete your data?")) {
		try {
			await deleteFormInfo("name");
			console.log("name deleted successfully");
			await deleteFormInfo("description");
			console.log("description deleted successfully");
			await deleteFormInfo("picture");
			console.log("description deleted successfully");
			localStorage.setItem("formSubmitted", "false");

			// Perform any additional actions after the form is deleted\
			window.location.reload();
		} catch (error) {
			console.error("Error deleting form:", error);
			// Handle the error or display an error message to the user
		}
	}
});
