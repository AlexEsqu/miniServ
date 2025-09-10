<!-- Hi <?php echo htmlspecialchars($_POST['name']); ?>.
You are <?php echo (int) $_POST['age']; ?> years old. -->

<?php
echo '<p>Your user agents is ' .$_SERVER['HTTP_USER_AGENT'] . ' 👀 </p>';
?>
