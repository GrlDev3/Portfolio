<div id="products">
<?php
//define("VARIABLE","jrUmrlbHRevgwhskwpLdpoxkksqe");
//require("db_config.php");

$sql = "SELECT * FROM products ORDER BY price DESC";

$result = mysqli_query($connection, $sql) or die(mysqli_error($connection));

echo "<table border=\"1\" align=\"center\">";
echo "<tr><th>Product name</th><th>Image</th><th>Description</th><th>Price</th></tr>";

if(mysqli_num_rows($result)>0)
{
    while ($record = mysqli_fetch_array($result))
        echo "<tr><td>$record[productname]</td><td><img src=\"images/$record[image]\" alt=\"$record[productname]\" width=\"100\" /></td>
        <td  width='400'>$record[description]</td><td>$record[price]</td></tr>";
}
else
    echo "There is no data in the database!";

echo "</table>";

mysqli_free_result($result);
mysqli_close($connection);

?>
</div>

