// Get all elements with class "tablist-button"
const tablistButtons = document.getElementsByClassName("tablist-button");

// Get all elements with class "presentation-article"
const presentationArticles = document.getElementsByClassName("presentation-article");

// Variable to track if the form has been submitted
let formSubmitted = false;

// Function to update the visibility of the presentation articles based on the clicked tab
function updateArticleVisibility(clickedButton) {
  // Loop through all presentation articles
  Array.from(presentationArticles).forEach((article) => {
    // If the article ID matches the clicked button's aria-controls attribute, remove the hidden attribute
    if (article.getAttribute("id") === clickedButton.getAttribute("aria-controls")) {
      article.removeAttribute("hidden");
    } else {
      // Otherwise, add the hidden attribute
      article.setAttribute("hidden", "hidden");
    }
  });
}

// Function to toggle the aria-selected attribute on the tablist buttons
function toggleTablistAriaSelection(clickedButton) {
  // Loop through all tablist buttons
  Array.from(tablistButtons).forEach((button) => {
    // If the button is the clicked button, set aria-selected to true, otherwise set it to false
    if (button === clickedButton) {
      button.setAttribute("aria-selected", "true");
    } else {
      button.setAttribute("aria-selected", "false");
    }
  });
}

// Function to handle tablist button click events
function tablistHandler(e) {
  // Store the clicked button in the clickedButton variable
  clickedButton = e.target;

  // Call the toggleTablistAriaSelection function with the clicked button
  toggleTablistAriaSelection(clickedButton);

  // Call the updateArticleVisibility function with the clicked button
  updateArticleVisibility(clickedButton);

  // Get the form buttons element
  const formButtons = document.getElementById("form-buttons");

  // If the clicked button's aria-controls attribute is "add-yourself-article", show the form buttons, otherwise hide them
  if (e.target.getAttribute("aria-controls") === "add-yourself-article") {
    formButtons.style.display = "inline";
  } else {
    formButtons.style.display = "none";
  }
}

// Get all elements with class "Close"
const crossButttons = document.getElementsByClassName("Close");

// Loop through all cross buttons and add a click event listener
Array.from(crossButttons).forEach((button) => {
  button.addEventListener("click", (e) => {
    // Hide the closest window element
    button.closest(".window").style.display = "none";
  });
});

// Loop through all tablist buttons and add a click event listener
Array.from(tablistButtons).forEach((button) => {
  button.addEventListener("click", (e) => tablistHandler(e));
});

// Get all elements with class "window"
const windows = document.getElementsByClassName("window");

// Variables to track the active window and the offset of the cursor from the window's top-left corner
let offsetX, offsetY, activeWindow = null, i = 1;

// Loop through all windows and add a mousedown event listener
for (let w of windows) {
  w.addEventListener("mousedown", (e) => {
    // Set the active window to the clicked window
    activeWindow = w;

    // Bring the active window to front (optional)
    activeWindow.style.zIndex = 10 + i;
    Array.from(activeWindow.getElementsByTagName("*")).forEach((child) => {
      child.style.zIndex = 10 + i;
    });

    // Calculate the offset between the cursor and the window's top-left corner
    offsetX = e.clientX - activeWindow.getBoundingClientRect().left;
    offsetY = e.clientY - activeWindow.getBoundingClientRect().top;

    // Add global listeners for mousemove and mouseup events
    document.addEventListener("mousemove", mouseMoveHandler);
    document.addEventListener("mouseup", mouseUpHandler);
    i++;
  });
}

// Function to handle mousemove events
function mouseMoveHandler(e) {
  // If no active window, return
  if (!activeWindow) return;

  // Set the window's position to absolute
  activeWindow.style.position = "absolute";

  // Set the window's left position to the cursor's x position minus the offset
  activeWindow.style.left = `${e.clientX - offsetX}px`;

  // Set the window's top position to the cursor's y position minus the offset
  activeWindow.style.top = `${e.clientY - offsetY}px`;
}

// Function to handle mouseup events
function mouseUpHandler() {
  // If no active window, return
  if (!activeWindow) return;

  // Reset the active window variable
  activeWindow = null;

  // Remove the mousemove and mouseup event listeners
  document.removeEventListener("mousemove", mouseMoveHandler);
  document.removeEventListener("mouseup", mouseUpHandler);
}

// Get the form element
const form = document.querySelector("form");

// Function to send the form data to the server
async function sendForm() {
  // Create a new FormData object with the form's data
  const formData = new FormData(form);

  try {
    // Send a POST request to the server with the form data
    const response = await fetch("/post", {
      method: "POST",
      body: formData,
    });

    // If the request was successful, log the response body
    if (response.ok) {
      console.log("Data:", response.body);
  	localStorage.setItem("formSubmitted", "true");
      // Display success message or perform any other action
    } else {
      // If the request was not successful, log the error status
      console.error("Error:", response.status);
    }
  } catch (error) {
    // Log any errors that occurred during the request
    console.error(error);
  }
}

// Function to get form information from the server
async function getFormInfo(endpoint, responseType = "text") {
  return new Promise((resolve, reject) => {
    // Create a new XMLHttpRequest object
    const req = new XMLHttpRequest();

    // Set up the request
    req.open("GET", "http://localhost:8080/post/" + endpoint);
    req.setRequestHeader("Session", "yes");
    req.responseType = responseType;

    // Handle the request's response
    req.onload = () => {
      if (req.readyState == 4 && req.status >= 200 && req.status < 300) {
        // If the request was successful, resolve the promise with the response data
        resolve(req.response);
		localStorage.setItem("formSubmitted", "true");

      } else {
        // If the request was not successful, reject the promise with an error
        reject(new Error(`Error: ${req.status}`));
      }
    };

    // Handle any errors that occurred during the request
    req.onerror = () => {
      reject(new Error("Network error"));
    };

    // Send the request
    req.send();
  });
}
// Function to get form data from the server
async function getFormData() {
  let name, description, picture;
  const article = document.getElementById("add-yourself-article");

  try {
    // Get the form data from the server
    name = await getFormInfo("name", "text");
    description = await getFormInfo("description", "text");
    picture = await getFormInfo("picture", "blob");

    // Create a URL for the picture data
    imageUrl = URL.createObjectURL(picture);

    // Set the name of the tablist button to the name retrieved from the server
    document.getElementById("tablist-button-add-yourself").textContent = name;

    // If the form has been submitted before, hide the form elements and display the retrieved data
    if (localStorage.getItem("formSubmitted") == "true") {
      for (let i = 0; i < article.children.length; i++) {
        article.children[i].style.display = "none";
      }
      document.getElementById("form-button-ok").style.display = "none";
      document.getElementById("form-button-cancel").style.display = "none";

      // If there is a picture, create an image element and set its source to the picture data
      if (picture) {
        const img = document.createElement("img");
        img.src = imageUrl;
        article.appendChild(img);
      }

      // If there is a description, create a paragraph element and set its text content to the description
      if (description) {
        const p = document.createElement("p");
        p.textContent = description;
        article.appendChild(p);
      }
    }
  } catch (error) {
    // If an error occurs, set the formSubmitted flag to false and log the error
    localStorage.setItem("formSubmitted", "false");
    console.error(error);
    return;
  }
}

// Add an event listener to the form's submit event
form.addEventListener("submit", async function (e) {
  e.preventDefault(); // Prevents the default form submission behavior
  // Send the form data to the server
  sendForm();
  // Reset the form
  form.reset();
  // Get the form data from the server
  		getFormData();

});

// Add an event listener to the window's load event
window.addEventListener("load", async (event) => {
  try {
    // Get the form data from the server when the window loads
    getFormData();
  } catch (error) {
    // If an error occurs, log the error
    console.log(error);
	console.log(hey);
  }
});

// Get the delete button element
var deleteButton = document.getElementById("delete-button");

async function deleteFormInfo(endpoint, responseType = "text") {
  return new Promise((resolve, reject) => {
    const req = new XMLHttpRequest();
    req.open("DELETE", "http://localhost:8080/post/" + endpoint);
    req.responseType = responseType;
    req.onload = () => {
      if (req.readyState == 4 && req.status >= 200 && req.status < 300) {
        // If the request was successful, resolve the promise with the response data
        resolve(req.response);
      } else {
        // If the request was not successful, reject the promise with an error
        reject(new Error(`Error: ${req.status}`));
      }
    };

    req.onerror = () => {
      // If an error occurs during the request, reject the promise with an error
      reject(new Error("Network error"));
    };

    // Send the request
    req.send();
  });
}

// Add a click event listener to the delete button
deleteButton.addEventListener("click", async (e) => {
  // If the user confirms the deletion, delete the form information from the server
  if (window.confirm("Are you sure you want to delete your data?")) {
    try {
      await deleteFormInfo("name");
      console.log("name deleted successfully");
      await deleteFormInfo("description");
      console.log("description deleted successfully");
      await deleteFormInfo("picture");
      console.log("picture deleted successfully");
      // Set the formSubmitted flag to false
      localStorage.setItem("formSubmitted", "false");

      // Reload the window to refresh the page
      window.location.reload();
    } catch (error) {
      // If an error occurs during the deletion, log the error
      console.error("Error deleting form:", error);
      // Handle the error or display an error message to the user
    }
  }
});