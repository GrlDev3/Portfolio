<?php
//define("VARIABLE","jrUmrlbHRevgwhskwpLdpoxkksqe");
//require('db_config.php');

$search = mysqli_real_escape_string($connection,$_GET['search']);

echo "<p>The keyword which you searched for, was: <b>".$search."</b>. The results: </p>";

$sql = "SELECT * FROM products
        WHERE Binary productname LIKE '%$search%'";

$result = mysqli_query($connection, $sql) or die(mysqli_error($connection));

echo "<table border=\"1\" align=\"center\">";
echo "<tr><th>Product name</th><th>Image</th><th>Description</th><th>Price</th></tr>";

if (mysqli_num_rows($result)>0)
{
    while ($row=mysqli_fetch_array($result,MYSQLI_BOTH))
        echo "<tr><td>".$row[1]."</td><td><img src=\"images/".$row[2]."\" alt=\"".$row[1]."\" width=\"100\" /></td><td>".$row[3]."</td><td>".$row[4]."</td>";
    mysqli_free_result($result);

}
else
    echo "<tr><td colspan=\"4\">There is no match for \"$search\" in the database! </td></tr> ";

echo "</table>";
mysqli_close($connection);
?>

