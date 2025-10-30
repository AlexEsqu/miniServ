<?php
header("Content-Type: text/plain");

if (!empty($_GET)) {
	echo "Received Parameters:\n";
	foreach ($_GET as $key => $value) {
		echo "$key: $value\n";
	}
} else {
	echo "No parameters received.\n";
}
?>
