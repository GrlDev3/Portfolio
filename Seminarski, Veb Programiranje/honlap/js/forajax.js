/* Ajax hasznalat*/

window.addEventListener('load', function() {
    //alert("window check");
    document.getElementById('search').addEventListener('keyup', Ajax);
});

    function putin(id) {
        //alert("putin check");
        document.getElementById('search').value = document.getElementById(id).innerHTML;
        document.getElementById('suggest').innerHTML=" ";
    }

    function Ajax() {
        //alert("ajax check");
        var keyword = document.getElementById('search').value;

        var xmlhttp;
        if (window.XMLHttpRequest) {// code for IE7+, Firefox, Chrome, Opera, Safari
            xmlhttp = new XMLHttpRequest();
        }
        else {// code for IE6, IE5
            xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
        }
        xmlhttp.onreadystatechange = function () {
            //alert(xmlhttp.readyState);
            if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {

                document.getElementById("suggest").innerHTML = xmlhttp.responseText;
            }
        }
        xmlhttp.open("GET", "ajax.php?keyword=" + keyword, true);
        xmlhttp.send();

    }