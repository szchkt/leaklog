function select(id) {
	var selected = document.getElementsByClassName("selected");
	for (var i = 0; i < selected.length; ++i)
		if (selected[i].id.slice(0, selected[i].id.indexOf(':')) == id.slice(0, id.indexOf(':')))
			selected[i].className = selected[i].className.replace(/(?:^|\s)selected(?!\S)/, '');
	document.getElementById(id).className += "selected";
}
function executeLink(tr, link) {
	if (tr.className.match(/(\s|^)selected(\s|$)/))
		window.location = link + "/edit";
	else
		window.location = link;
}
