<?php

echo "Environment Variables:\n";
print_r($_SERVER);

echo "QUERY_STRING: " . $_SERVER['QUERY_STRING'] . "\n";

print_r($_GET);

// Check if there are query parameters
if (!empty($_GET)) {
	echo "Received Parameters:\n";
	foreach ($_GET as $key => $value) {
		echo "$key: $value\n";
	}
} else {
	echo "No parameters received.\n";
}
?>
