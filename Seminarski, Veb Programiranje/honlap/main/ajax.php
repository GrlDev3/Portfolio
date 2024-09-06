
<?php
define("VARIABLE","jrUmrlbHRevgwhskwpLdpoxkksqe");
require ("../db_config.php");
$keyword=mysqli_real_escape_string($connection,$_GET['keyword']);
if(empty(trim($keyword))){
    return "";
}
$sql="SELECT productname,id_product from products WHERE productname LIKE '$keyword%'";
$res=mysqli_query($connection,$sql);
echo"<table>";
while($row=mysqli_fetch_array($res)){
    echo"<tr><td id='$row[1]' onclick='putin($row[1])'>$row[0]</td></tr>";
}
echo "</table>";