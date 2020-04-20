/* 
   Javascript file for WiFimanager_for_Multidosplay -> must be placed on SPIFFS file system
   
*/


function c(l){ 
document.getElementById('s').value=l.innerText||l.textContent; 
p = l.nextElementSibling.classList.contains('l'); 
document.getElementById('p').disabled = !p; 
if(p)document.getElementById('p').focus();
}

function hideStatic() {
		document.getElementById('iplabel').style.display = 'none';
		document.getElementById('ip').style.display = 'none';
		document.getElementById('gwlabel').style.display = 'none';
		document.getElementById('gw').style.display = 'none';
		document.getElementById('snlabel').style.display = 'none';
		document.getElementById('sn').style.display = 'none';
		document.getElementById('dnslabel').style.display = 'none';
		document.getElementById('dns').style.display = 'none';
}

function showStatic() {
		document.getElementById('iplabel').style.display = 'block';
		document.getElementById('ip').style.display = 'block';
		document.getElementById('gwlabel').style.display = 'block';
		document.getElementById('gw').style.display = 'block';
		document.getElementById('snlabel').style.display = 'block';
		document.getElementById('sn').style.display = 'block';
		document.getElementById('dnslabel').style.display = 'block';
		document.getElementById('dns').style.display = 'block';
}

function staticCheck() {
    if (document.getElementById('dhcp').checked == true) {
        hideStatic();
    } else {
		showStatic();
	}
}