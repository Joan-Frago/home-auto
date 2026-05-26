<?php
require_once "util.php";

function replace_templates($template_path, $svg_path, $id_device, $device_type, $width, $height){
	$template = file_get_contents($template_path);
	$svg = file_get_contents($svg_path);

	$svg = str_replace(
		["#@ID_DEVICE@#","#@DEVICE_TYPE@#"],
		[$id_device, $device_type],
		$svg
	);

	$template = str_replace(
		["#@ID_DEVICE@#","#@DEVICE_TYPE@#", "#@SVG@#", "#@WIDTH@#", "#@HEIGHT@#"],
		[$id_device, $device_type, $svg, $width, $height],
		$template
	);

	return $template;
}

function insert_svg($svg_path, $id_device, $device_type, $width, $height){
	$template_path = "templates/index.php.html";
	$section_str = replace_templates($template_path, $svg_path, "device_".$id_device, $device_type, $width, $height);

	echo $section_str;
}

?>
