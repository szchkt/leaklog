declare variable $inputDocument external;
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Customers</title>
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
<table cellspacing="0" cellpadding="4" style="width:100%;">
{
let $d := doc($inputDocument)

return (
	for $c in $d/leaklog/customers/customer
	return (
		<tr style="background-color: #eee;"><td colspan="2" style="font-size: large; text-align: center;"><b><i18n>Company: </i18n>
		<a href="customer:{ data($c/@id) }">{data($c/@company)}</a></b></td></tr>,
		<tr><td><table cellspacing="0" cellpadding="4" style="width:100%;">
		<tr><td style="text-align: right; width:50%;"><i18n>ID: </i18n></td><td> {
			data($c/@id)
		}</td></tr>
		<tr><td style="text-align: right; width:50%;"><b><i18n>Contact person: </i18n></b></td><td><b> {
			data($c/@name)
		}</b></td></tr>
		<tr><td style="text-align: right; width:50%;"><i18n>Address: </i18n></td><td> {
			data($c/@address)
		}</td></tr>
		<tr><td style="text-align: right; width:50%;"><i18n>E-mail: </i18n></td><td> {
			data($c/@mail)
		}</td></tr>
		</table></td>
		<td><table cellspacing="0" cellpadding="4" style="width:100%;">
		<tr><td style="text-align: right; width:50%;"><i18n>Phone: </i18n></td><td> {
			data($c/@phone)
		}</td></tr>
		<tr><td style="text-align: right; width:50%;"><i18n>Number of circuits: </i18n></td><td> {
			count($c/circuit)
		}</td></tr>
		<tr><td style="text-align: right; width:50%;"><i18n>Total number of inspections: </i18n></td><td> {
			let $x := for $i in $c/circuit return count($i/inspection)
			return sum($x)
		}</td></tr>
		</table></td></tr>
	)
)
}
</table>
</body>
</html>
