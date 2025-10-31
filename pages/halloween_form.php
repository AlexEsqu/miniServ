<?php
header("Content-Type: text/html");
?>
<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>🎃 Halloween Survey 🎃</title>
	<style>
		body {
			background-color: #1a1a1a;
			color: #ff6600;
			font-family: 'Courier New', monospace;
			text-align: center;
			padding: 50px;
		}
		.container {
			max-width: 600px;
			margin: 0 auto;
			background-color: #2d2d2d;
			padding: 40px;
			border: 3px solid #ff6600;
			border-radius: 15px;
			box-shadow: 0 0 20px #ff6600;
		}
		h1 {
			font-size: 3em;
			text-shadow: 2px 2px 4px #000;
			margin-bottom: 30px;
		}
		.skull {
			font-size: 5em;
			animation: float 3s ease-in-out infinite;
		}
		@keyframes float {
			0%, 100% { transform: translateY(0); }
			50% { transform: translateY(-20px); }
		}
		label {
			display: block;
			font-size: 1.2em;
			margin: 20px 0 10px;
			text-align: left;
		}
		input[type="text"] {
			width: 100%;
			padding: 15px;
			font-size: 1em;
			background-color: #1a1a1a;
			color: #ff6600;
			border: 2px solid #ff6600;
			border-radius: 5px;
			box-sizing: border-box;
		}
		input[type="submit"] {
			margin-top: 30px;
			padding: 15px 40px;
			font-size: 1.3em;
			background-color: #ff6600;
			color: #1a1a1a;
			border: none;
			border-radius: 10px;
			cursor: pointer;
			font-weight: bold;
			transition: all 0.3s;
		}
		input[type="submit"]:hover {
			background-color: #ff9900;
			transform: scale(1.1);
			box-shadow: 0 0 20px #ff6600;
		}
		.warning {
			color: #ff0000;
			font-style: italic;
			margin-top: 20px;
		}
	</style>
</head>
<body>
	<div class="container">
		<div class="skull">💀</div>
		<h1>🎃 Halloween Survey 🎃</h1>
		<p style="font-size: 1.2em; margin-bottom: 30px;">
			Dare to reveal your darkest secrets...
		</p>

		<form method="GET" action="halloween_result.php">
			<label for="fear">👻 What is your WORST FEAR?</label>
			<input type="text" id="fear" name="fear" required
				placeholder="Spiders? Ghosts? Clowns?">

			<label for="candy">🍬 What is your FAVORITE CANDY?</label>
			<input type="text" id="candy" name="candy" required
				placeholder="Reese's? Snickers? Candy Corn?">

			<input type="submit" value="🎃 SUBMIT 🎃">
		</form>

		<p class="warning">⚠️ The spirits are watching... ⚠️</p>
	</div>
</body>
</html>
