<?php
	require_once "home.php";
?>

<!DOCTYPE html>
<html lang="es">
<head>
	<meta charset="UTF-8">
	<title>Home Control</title>
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<link rel="stylesheet" href="./home.css">
	<script src="./home.js"></script>
</head>
<body>
	<header class="header">
		<h1>Control Panel</h1>
	</header>
	<main class="house-grid">

		<section class="device">
			<div class="device-svg">
				<?php insert_svg("svg/persiana.svg"); ?>
			</div>
			<h2>Persiana Dormitorio</h2>
			<button class="device-btn off">Bajada</button>
		</section>

	</main>
	
	<div class="invisible">
		<?php get_all_devices(); ?>
	</div>

	<script>
		load_pin_data();
	</script>
</body>
</html>
