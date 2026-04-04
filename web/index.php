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

		<?php insert_svg(svg_path: "svg/persiana.svg", 			id_device: 1, device_type: "BLIND",        width: 160, height: 160); ?>
		<?php insert_svg(svg_path: "svg/bulb.svg",     			id_device: 2, device_type: "LIGHT_BULB",   width: 120, height: 170); ?>
		<?php insert_svg(svg_path: "svg/Circutor-CVM-MINI.svg",	id_device: 3, device_type: "ANALYZER",     width: 200, height: 240); ?>
		<?php insert_svg(svg_path: "svg/water-heater.svg",     	id_device: 4, device_type: "WATER_HEATER", width: 101, height: 308); ?>

	</main>
	
	<div class="invisible">
	</div>

	<script>
		set_all_devices();
		load_pin_data();
	</script>
</body>
</html>
