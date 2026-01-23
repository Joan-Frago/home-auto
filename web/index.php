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
	<script src="./home.js" defer></script>
</head>
<body>
	<header class="header">
		<h1>Control Panel</h1>
	</header>
	<main class="house-grid">

		<!-- Elemento: Luz -->
		<section class="device">
			<div class="device-svg">
				<?php //insert_svg("svg/luz.svg"); ?>
			</div>
			<h2>Luz Salón</h2>
			<button class="device-btn on">Encendida</button>
		</section>

		<!-- Elemento: Persiana -->
		<section class="device">
			<div class="device-svg">
				<?php insert_svg("svg/persiana.svg"); ?>
			</div>
			<h2>Persiana Dormitorio</h2>
			<button class="device-btn off">Bajada</button>
		</section>

		<!-- Elemento: Caldera -->
		<section class="device">
			<div class="device-svg">
				<?php //insert_svg("svg/caldera.svg"); ?>
			</div>
			<h2>Caldera</h2>
			<button class="device-btn on">Activa</button>
		</section>

		<!-- Elemento: Calefacción -->
		<section class="device">
			<div class="device-svg">
				<?php //insert_svg("svg/calefaccion.svg"); ?>
			</div>
			<h2>Calefacción</h2>
			<button class="device-btn off">Apagada</button>
		</section>
	</main>
	
	<div class="invisible">
		<?php get_all_devices(); ?>
	</div>
</body>
</html>
