<?php
header("Content-Type: text/html");

// Get the form data
$fear = isset($_GET['fear']) ? htmlspecialchars($_GET['fear']) : 'the unknown';
$candy = isset($_GET['candy']) ? htmlspecialchars($_GET['candy']) : 'mystery candy';

// Array of spooky responses
$spookyMessages = array(
	"The spirits have heard your confession...",
	"Your secrets echo through the haunted halls...",
	"The darkness knows your truth...",
	"The ghosts are pleased with your honesty...",
	"Your fears feed the shadows..."
);

$candyMessages = array(
	"A wise choice for your final treat...",
	"The witches approve of your selection...",
	"Such sweet temptation...",
	"The perfect offering to the spirits...",
	"Delicious... the vampires concur..."
);

// Pick random messages
$randomSpooky = $spookyMessages[array_rand($spookyMessages)];
$randomCandy = $candyMessages[array_rand($candyMessages)];
?>
<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>👻 Your Halloween Destiny 👻</title>
	<style>
		body {
			background-color: #0a0a0a;
			color: #ff6600;
			font-family: 'Courier New', monospace;
			text-align: center;
			padding: 50px;
			animation: fadeIn 2s;
		}
		@keyframes fadeIn {
			from { opacity: 0; }
			to { opacity: 1; }
		}
		.container {
			max-width: 700px;
			margin: 0 auto;
			background-color: #1a1a1a;
			padding: 40px;
			border: 3px solid #ff0000;
			border-radius: 15px;
			box-shadow: 0 0 30px #ff0000;
		}
		h1 {
			font-size: 2.5em;
			text-shadow: 3px 3px 6px #000;
			margin-bottom: 30px;
			animation: pulse 2s infinite;
		}
		@keyframes pulse {
			0%, 100% { opacity: 1; }
			50% { opacity: 0.7; }
		}
		.ghost {
			font-size: 6em;
			animation: float 3s ease-in-out infinite;
		}
		@keyframes float {
			0%, 100% { transform: translateY(0) rotate(0deg); }
			50% { transform: translateY(-30px) rotate(10deg); }
		}
		.result-box {
			background-color: #2d2d2d;
			padding: 30px;
			margin: 20px 0;
			border: 2px solid #ff6600;
			border-radius: 10px;
			text-align: left;
		}
		.result-label {
			font-size: 1.5em;
			color: #ff0000;
			font-weight: bold;
			margin-bottom: 10px;
		}
		.result-value {
			font-size: 1.8em;
			color: #ffff00;
			margin-bottom: 15px;
			text-shadow: 2px 2px 4px #000;
		}
		.message {
			font-style: italic;
			color: #cccccc;
			font-size: 1.1em;
		}
		.back-button {
			display: inline-block;
			margin-top: 30px;
			padding: 15px 40px;
			font-size: 1.2em;
			background-color: #ff6600;
			color: #1a1a1a;
			text-decoration: none;
			border-radius: 10px;
			font-weight: bold;
			transition: all 0.3s;
		}
		.back-button:hover {
			background-color: #ff9900;
			transform: scale(1.1);
			box-shadow: 0 0 20px #ff6600;
		}
		.warning {
			color: #ff0000;
			font-size: 1.5em;
			margin-top: 30px;
			animation: blink 1s infinite;
		}
		@keyframes blink {
			0%, 50%, 100% { opacity: 1; }
			25%, 75% { opacity: 0; }
		}
	</style>
</head>
<body>
	<div class="container">
		<div class="ghost">👻</div>
		<h1>🕷️ YOUR HALLOWEEN DESTINY 🕷️</h1>

		<div class="result-box">
			<div class="result-label">💀 YOUR WORST FEAR:</div>
			<div class="result-value"><?php echo $fear; ?></div>
			<div class="message"><?php echo $randomSpooky; ?></div>
		</div>

		<div class="result-box">
			<div class="result-label">🍭 YOUR FAVORITE CANDY:</div>
			<div class="result-value"><?php echo $candy; ?></div>
			<div class="message"><?php echo $randomCandy; ?></div>
		</div>

		<p class="warning">⚠️ YOUR SECRETS ARE SAFE... FOR NOW ⚠️</p>

		<a href="halloween_form.php" class="back-button">🎃 HAUNT AGAIN 🎃</a>
	</div>
</body>
</html>
