declare variable $inputDocument external;
declare function local:returnAnyVar ($var as element(), $i as element()) {
	for $ev in $var/value/ec
		return
			if (count($ev/@f)) then (
				data($ev/@f)
			) else if (count($ev/@sum)) then (
				for $sum in $i/../inspection[substring-before($i/@date, '.') = substring-before(@date, '.')]
					return ( string("+"), data($sum/var/var[@id=$ev/@sum]) )
			) else if (count($ev/@cc_attr)) then (
				for $att in $i/../@*
					return (
					if (name($att) = data($ev/@cc_attr)) then (
							data($att)
					) else ()
				)
			) else (
				if (empty($i/var[@id=$ev/@id])) then (
					string($i/var/var[@id=$ev/@id])
				) else (
					string($i/var[@id=$ev/@id])
				)
			)
};

declare function local:returnFootExpression ($var as element(), $circuit as element(), $begin as xs:integer)  {
	<remove_expr>{
	for $i in $circuit/inspection[xs:integer(substring-before(@date, '.')) >= $begin]
		return
		<expr date="{
				data($i/@date)
			}">{
			string("+("),
			local:returnAnyVar ($var, $i),
			string(")")
		}</expr>
	}</remove_expr>
};

declare function local:returnExpression ($table as element(), $v_vars as element(), $x as element())  {
	<expression>{
								if (data($table/@highlight_nominal)="false") then ()
								else (
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
									) else ()
								),
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

declare function local:setTableHead ($table as element(), $vars as element())  {

<tr class="border_top">
<th rowspan="3">Dátum</th>
{
for $i in $table/var
	let $s := count($vars/var[@id=$i/@id]/var)
	return <th colspan="{$s}" rowspan="{
			if ($s > 1) then xs:integer(1)
			else 2+xs:integer(empty($vars/var[@id=$i/@id]/@unit))
		}" class="{ data($vars/var[@id=$i/@id]/@col_bg) }">{data($vars/var[@id=$i/@id]/@name)}</th>
}
</tr>,
<tr>
{
for $i in $table/var
	return
		if (count($vars/var[@id=$i/@id]/var)) then (
			for $z in $vars/var[@id=$i/@id]/var
				return <th rowspan="{
							if (xs:integer(empty($z/@unit)) = 1) then (
								2
							) else (1)
							}" class="{ data($vars/var[@id=$i/@id]/@col_bg) }">{data($z/@name)}</th>
		)
		else ()
}
</tr>,
<tr class="border_bottom">
{
for $i in $table/var
	return
		if (count($vars/var[@id=$i/@id]/var)) then (
			for $z in $vars/var[@id=$i/@id]/var
				return
					if (empty($z/@unit)) then ()
					else (
						<th class="{ data($vars/var[@id=$i/@id]/@col_bg) }">{data($z/@unit)}</th>
					)
		)
		else (
			if (empty($vars/var[@id=$i/@id]/@unit)) then ()
			else <th class="{ data($vars/var[@id=$i/@id]/@col_bg) }">{data($vars/var[@id=$i/@id]/@unit)}</th>
		)
}
</tr>
};

declare function local:setTableBody ($table as element(), $circuit as element(), $vars as element(), $begin as xs:integer)  {

for $x in $circuit/inspection[(not($table/@highlight_nominal = "false") and @nominal = "true") or xs:integer(substring-before(@date, '.')) >= $begin]
	return <tr class="{
			if (data($table/@highlight_nominal)="false") then ()
			else if ($x/@nominal="true") then (xs:string("nominal"))
			else ()
		}">{
		<td id="date"><a href="customer:{ data($circuit/../@id) }/circuit:{ data($circuit/@id) }/inspection:{ data($x/@date) }">{data($x/@date)}</a></td>,
		for $y in $table/var
			return
				if (count($vars/var[@id=$y/@id]/var)) then (
					for $z in $vars/var[@id=$y/@id]/var
						return (
						<td id="{
							data($z/@id)
						}" remove="{
								if (empty($vars/var[@id=$y/@id]/var[@id=$z/@id]/value/ec/@sum)) then ()
								else ( substring-before($x/@date, '.') )
						}" class="{ data($vars/var[@id=$y/@id]/@col_bg) }" rowspan="{
							if (empty($vars/var[@id=$y/@id]/var[@id=$z/@id]/value/ec/@sum)) then (
								1
							) else (
								if (substring-before($x/@date, '.') = substring-before($x/../inspection[position()=$x/position()-1]/@date, '.')) then (
									1
								) else (
									count($x/../inspection[substring-before($x/@date, '.') = substring-before(@date, '.')])
								)
							)
						}">{
							if (empty($z/value)) then (
								if (data($table/@highlight_nominal)="false") then ()
								else if (data($z/@compare_nom)="true") then (
									<expression>{
									data($x/../inspection[@nominal="true"]/var[@id=$y/@id]/var[@id=$z/@id]),
									string("?"),
									data($x/var[@id=$y/@id]/var[@id=$z/@id])
									}</expression>
								)
								else data($x/var[@id=$y/@id]/var[@id=$z/@id])
							)
							else local:returnExpression ($table, $z, $x)
						}</td>
						)
				)
				else (<td id="{
							data($y/@id)
						}" class="{ data($vars/var[@id=$y/@id]/@col_bg) }">{
							if (empty($vars/var[@id=$y/@id]/value)) then (
								if (data($table/@highlight_nominal)="false") then ()
								else if (data($vars/var[@id=$y/@id]/@compare_nom)="true") then (
									<expression>{
									data($x/../inspection[@nominal="true"]/var[@id=$y/@id]),
									string("?"),
									data($x/var[@id=$y/@id])
									}</expression>
								)
								else data($x/var[@id=$y/@id])
							)
							else local:returnExpression ($table, $vars/var[@id=$y/@id], $x)
						}</td>)
		}</tr>
};

declare function local:setTableFoot ($table as element(), $circuit as element(), $vars as element(), $begin as xs:integer) {
	if ($table/foot) then (
		<tr class="border_top border_bottom">{
			<td>{data($table/foot/@name)}</td>,
			for $i in $table/var
				let $var := $vars/var[@id=$i/@id]
				return
					if ($table/foot/var[@id=$i/@id]) then (
						let $f := $table/foot/var[@id=$i/@id]
						return
							if (count($var/var)) then (
								for $v in $vars/var[@id=$i/@id]/var
									return
										<td class="{ data($var/@col_bg) }">{
										if (data($f/@function)="sum") then (
											if (count($v/value/ec/@sum)) then (
												local:returnFootExpression($v, $circuit, $begin)
											) else (
												let $s := for $z in $circuit/inspection[(not($table/@highlight_nominal = "false") and @nominal = "true") or xs:integer(substring-before(@date, '.')) >= $begin]
													return $z/var[@id=$f/@id]/var[@id=$v/@id]
												return sum($s)
											)
										) else ()
										}</td>
							) else (
								if (data($f/@function)="sum") then (
									let $s := for $z in $circuit/inspection[(not($table/@highlight_nominal = "false") and @nominal = "true") or xs:integer(substring-before(@date, '.')) >= $begin]
										return $z/var[@id=$f/@id]
									return <td class="{ data($var/@col_bg) }">{sum($s)}</td>
								) else <td></td>
							)
					) else (
						if (count($var/var)) then (
							for $v in $var/var
								return <td class="{ data($v/../@col_bg) }"></td>
						) else (
							<td class="{ data($var/@col_bg) }"></td>
						)
					)
		}
		</tr>
	)
	else ()
};

declare function local:setTopTable ($customer as element()) {
	<table><tr>
		<th>
			<i18n>ID</i18n>
		</th>
		<th>
			<i18n>Company</i18n>
		</th>
		<th>
			<i18n>Contact person</i18n>
		</th>
		<th>
			<i18n>Address</i18n>
		</th>
		<th>
			<i18n>E-mail</i18n>
		</th>
		<th>
			<i18n>Phone</i18n>
		</th>
	</tr><tr>
		<td>{
			data($customer/@id)
		}</td>
		<td>{
			data($customer/@company)
		}</td>
		<td>{
			data($customer/@name)
		}</td>
		<td>{
			data($customer/@address)
		}</td>
		<td>{
			data($customer/@mail)
		}</td>
		<td>{
			data($customer/@phone)
		}</td>
	</tr></table>
};

<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Table</title>
<link href="default.css" rel="stylesheet" type="text/css" />
<link href="colours.css" rel="stylesheet" type="text/css" />
<script type="text/javascript" src="shared.js"></script>
<script type="text/javascript">
<!--
%1
-->
</script>
</head>
<body onLoad="onTableLoad(); translate();">
{
let $d := doc($inputDocument)
let $circuit := $d/leaklog/customers/customer[@id="%2"]/circuit[@id="%3"]
let $table := $d/leaklog/tables/table[@id="%4"]
let $begin := %5
let $vars := $d/leaklog/variables
return (

local:setTopTable ($circuit/..),
<br />,

if (count($vars)) then (
<table>
<thead>
{
	local:setTableHead ($table, $vars)
}
</thead>
<tbody id="main_table_body">
{
	local:setTableBody ($table, $circuit, $vars, $begin)
}
</tbody>
<tfoot id="table_foot">
{
	local:setTableFoot ($table, $circuit, $vars, $begin)
}
</tfoot>
<tfoot id="poruchy_element">
<tr><td style="text-align: center;"><b><i18n>Warnings</i18n></b></td></tr>
{
	$d/leaklog/warnings
}
</tfoot>
</table>
) else ()
)
}
</body>
</html>
