declare variable $inputDocument external;
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Customer</title>
<style type="text/css">
<!--
body,td,th {
	font-family: "Lucida Grande", "Lucida Sans Unicode", verdana, lucida, sans-serif;
	font-size: small;
	color: #333333;
}
a:link {
	color: #333333;
	text-decoration: none;
}
a:visited {
	color: #333333;
	text-decoration: none;
}
a:hover {
	color: #660000;
	text-decoration: underline;
}
a:active {
	color: #333333;
	text-decoration: none;
}
-->
</style>
<script type="text/javascript">
<!--
%1
-->
</script>
</head>
<body onLoad="translate();">

{
let $d := doc($inputDocument)
let $c := $d/leaklog/customers/customer[@id="%2"]

return (
	<table cellspacing="0" cellpadding="4" style="width:100%;">
	<tr style="background-color: #DFDFDF;"><td colspan="2" style="font-size: larger; width:100%; text-align: center;"><b><i18n>Company: </i18n>
	<a href="customer:{ data($c/@id) }/modify">{data($c/@company)}</a></b></td></tr>
	<tr><td><table cellspacing="0" cellpadding="4" style="width:100%;">
	<tr><td style="text-align: right; width:50%;"><i18n>ID: </i18n></td><td style="width:50%;"> {
		data($c/@id)
	}</td></tr>
	<tr><td style="text-align: right;"><b><i18n>Contact person: </i18n></b></td><td><b> {
		data($c/@name)
	}</b></td></tr>
	<tr><td style="text-align: right;"><i18n>Address: </i18n></td><td> {
		data($c/@address)
	}</td></tr>
	<tr><td style="text-align: right;"><i18n>E-mail: </i18n></td><td> {
		data($c/@mail)
	}</td></tr>
	</table></td>
	<td><table cellspacing="0" cellpadding="4" style="width:100%;">
	<tr><td style="text-align: right; width:50%;"><i18n>Phone: </i18n></td><td style="width:50%;"> {
		data($c/@phone)
	}</td></tr>
	<tr><td style="text-align: right;"><i18n>Number of circuits: </i18n></td><td> {
		count($c/circuit)
	}</td></tr>
	<tr><td style="text-align: right;"><i18n>Total number of inspections: </i18n></td><td> {
		let $x := for $i in $c/circuit return count($i/inspection)
		return sum($x)
	}</td></tr>
	</table></td></tr>
	</table>,
	for $t in $c/circuit
		return (
			<table cellspacing="0" cellpadding="4" style="width:100%;"><tr><td rowspan="8" style="width:10%;"/>
			<td colspan="2" style="background-color: #eee; font-size: medium; text-align: center; width:80%;"><b><i18n>Circuit: </i18n>
			<a href="customer:{ data($c/@id) }/circuit:{ data($t/@id) }">
			{data($t/@id)}</a></b></td><td rowspan="8" style="width:10%;"/></tr>
			<tr><td><table cellspacing="0" cellpadding="4" style="width:100%;">
			<tr><td style="text-align: right; width:50%;"><b><i18n>Manufacturer: </i18n></b></td><td style="width:50%;"><b> {
				data($t/@manufacturer)
			}</b></td></tr>
			<tr><td style="text-align: right;"><i18n>Type: </i18n></td><td> {
				data($t/@type)
			}</td></tr>
			<tr><td style="text-align: right;"><i18n>Serial number: </i18n></td><td> {
				data($t/@sn)
			}</td></tr>
			<tr><td style="text-align: right; width:50%;"><i18n>Year of purchase: </i18n></td><td> {
				data($t/@year)
			}</td></tr>
			<tr><td style="text-align: right; width:50%;"><i18n>Date of commissioning: </i18n></td><td style="width:50%;"> {
				data($t/@commissioning)
			}</td></tr>
			<tr><td style="text-align: right; width:50%;"><i18n>Field of application: </i18n></td><td style="width:50%;"> {
				data($t/@field)
			}</td></tr>
			</table></td>
			<td><table cellspacing="0" cellpadding="4" style="width:100%;">
			<tr><td style="text-align: right; width:50%;"><i18n>Refrigerant: </i18n></td><td style="width:50%;"> {
				data($t/@refrigerant)
			}</td></tr>
			<tr><td style="text-align: right;"><i18n>Amount of refrigerant: </i18n></td><td> {
				data($t/@refrigerant_amount),
				string("l")
			}</td></tr>
			<tr><td style="text-align: right;"><i18n>Oil: </i18n></td><td> {
				data($t/@oil)
			}</td></tr>
			<tr><td style="text-align: right;"><i18n>Amount of oil: </i18n></td><td> {
				data($t/@oil_amount),
				string("kg")
			}</td></tr>
			<tr><td style="text-align: right;"><i18n>Service life: </i18n></td><td> {
				data($t/@life),
				<i18n> years</i18n>
			}</td></tr>
			<tr><td style="text-align: right;"><i18n>Run-time per day: </i18n></td><td> {
				data($t/@runtime),
				<i18n> hours</i18n>
			}</td></tr>
			<tr><td style="text-align: right;"><i18n>Rate of utilisation: </i18n></td><td> {
				data($t/@utilisation),
				string("%")
			}</td></tr>
			</table></td></tr>
			</table>
		)
)
}

</body>
</html>
