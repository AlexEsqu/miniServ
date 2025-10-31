<?php
header("Content-Type: text/plain");
echo "POST Data:\n";
foreach ($_POST as $key => $value) {
	echo "$key: $value\n";
}
?>
