<?php
set_time_limit(0);

echo "Starting infinite wait script...\n";
flush();

$counter = 0;
while (true) {
	sleep(5);
	$counter++;
	echo "Still waiting... (iteration: $counter)\n";
	flush();
}
?>
