declare variable $inputDocument external;
declare function local:returnExpression ($v_vars as element(), $x as element())  {
	<expression>{
								if (data($v_vars/@compare_nom)="true") then (
											for $ev in $v_vars/value/ec
											return (
												if (count($ev/@id)) then (
													if (empty($x/../inspection[@nominal="true"]/var[@id=$ev/@id])) then (
														if (empty($x/../inspection[@nominal="true"]/var/var[@id=$ev/@id])) then (0)
														else string($x/../inspection[@nominal="true"]/var/var[@id=$ev/@id])
													) else (
														string($x/../inspection[@nominal="true"]/var[@id=$ev/@id])
													)
												) else (
													data($ev/@f)
												)
										),
										string("?")
									) else (),
									for $ev in $v_vars/value/ec
									return
										if (count($ev/@f)) then (
											data($ev/@f)
										) else if (count($ev/@sum)) then (
											for $sum in $x/../inspection[substring-before($x/@date, '.') = substring-before(@date, '.')]
											return ( if (empty($sum/var/var[@id=$ev/@sum])) then ()
													else string("+"), data($sum/var/var[@id=$ev/@sum]) )
										) else if (count($ev/@cc_attr)) then (
											for $att in $x/../@*
											return (
												if (name($att) = data($ev/@cc_attr)) then (
													data($att)
												) else ()
											)
										) else (
											if (empty($x/var[@id=$ev/@id])) then (
												if (empty($x/var/var[@id=$ev/@id])) then (0)
												else string($x/var/var[@id=$ev/@id])
											) else (
												string($x/var[@id=$ev/@id])
											)
										)
	}</expression>
};

declare function local:returnSubVar (  $i as element(), $v as element())  {

  let $iv := $i/var[@id=$v/@id]
  for $w in $v/var
		return (
		if (empty($iv/var[@id=$w/@id]) and not(count($w/value))) then ()
		else (
			let $iw := $iv/var[@id=$w/@id]
			return <tr><td style="text-align: right; width:50%;">{
						data($v/@name),
						<i18n>: </i18n>,
						if (empty(data($w/@name))) then (
							data($w/@id)
						) else ( data($w/@name) ),
						<i18n>: </i18n>
					}</td><td id="{ data($w/@id) }">{
						if (empty($w/value)) then (
								if (data($w/@compare_nom)="true") then (
									<expression>{
									data($i/../inspection[@nominal="true"]/var[@id=$v/@id]/var[@id=$w/@id]),
									string("?"),
									data($iw)
									}</expression>
								)
								else data($iw)
							)
							else ( local:returnExpression ($w, $i) ),
							if (empty($w/@unit)) then ()
							else data($w/@unit)
						}</td></tr>
				))
 };

declare function local:returnVar ($i as element(), $v as element())  {

	let $iv := $i/var[@id=$v/@id]
	return if (empty($iv) and not(count($v/value))) then ()
	else (
					<tr><td style="text-align: right; width:50%;">{
						data($v/@name),
						<i18n>: </i18n>
						}</td><td id="{ data($v/@id) }">{
							if (empty($v/value)) then (
								if (data($v/@compare_nom)="true") then (
									<expression>{
									data($i/../inspection[@nominal="true"]/var[@id=$v/@id]),
									string("?"),
									data($iv)
									}</expression>
								)
								else data($iv)
							)
							else ( local:returnExpression ($v, $i) ),
							if (empty($v/@unit)) then ()
							else data($v/@unit)

				}</td></tr>
	)
};

declare function local:returnAnyVar ($i as element(), $v as element())  {

	if (count($v/var)) then (
			local:returnSubVar($i, $v)
		) else (
			local:returnVar($i, $v)
		)
};

<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Inspection</title>
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
<script type="text/javascript" src="shared.js">
<!--
%1
-->
</script>
</head>
<body onLoad="onInspectionLoad(); translate();">

<table cellspacing="0" style="width:100%;">
{
let $d := doc($inputDocument)

let $i := $d/leaklog/customers/customer[@id="%2"]/circuit[@id="%3"]/inspection[@date="%4"]
let $vars := $d/leaklog/variables
return (
	<tr><td><table cellspacing="0" cellpadding="4" style="width:100%;">
	<tbody id="main_table_body">
	<tr style="background-color: #eee;"><td colspan="2" style="font-size: larger; width:100%; text-align: center;"><b><i18n>Circuit: </i18n>
	<a href="customer:{ data($i/../../@id) }/circuit:{ data($i/../@id) }">{data($i/../@id)}</a></b></td></tr>
	<tr style="background-color: #DFDFDF;"><td colspan="2" style="font-size: larger; width:100%; text-align: center;"><b>{
			if (data($i/@nominal)="true") then <i18n>Nominal inspection: </i18n>
			else <i18n>Inspection: </i18n>
		}<a href="customer:{ data($i/../../@id) }/circuit:{ data($i/../@id) }/inspection:{ data($i/@date) }/modify">
	{data($i/@date)}</a></b></td></tr>
	<tr><td><table id="table_l" cellspacing="0" cellpadding="4" style="width:100%;">{
	if (count($vars/var)) then (
	for $v in $vars/var
	return (
		local:returnAnyVar($i, $v)
	)
	) else ()
	}</table></td>
	<td style="width:50%;"><table id="table_r" cellspacing="0" cellpadding="4" style="width:100%;">
	</table></td></tr>
	</tbody>
	</table></td></tr>,
	<tr><td>
	<table cellspacing="0" cellpadding="4" style="width:100%;" id="poruchy_element">
	<tr><td colspan="2" style="font-size: larger; width:100%;">
	<b><i18n>Warnings</i18n></b></td></tr>
	{$d/leaklog/warnings}
	</table></td></tr>
)
}
</table>
</body>
</html>
