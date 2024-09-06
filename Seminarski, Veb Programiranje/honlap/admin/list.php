<div>
    <h1>List of products</h1>
    <?php
    $sql = "SELECT * FROM products ORDER BY productname ASC";

    $result = mysqli_query($connection,$sql) or die(mysqli_error($connection));

    if(mysqli_num_rows($result)>0)
    {
        $no=1;
        echo '<table align="center">';
        echo '<tr><th>No.</th><th>Product name</th><th>Product image</th><th class="description">Description</th><th>Price</th><th>Operation</th></tr>';
        while ($record=mysqli_fetch_array($result,MYSQLI_BOTH))
        {
            echo "<tr>";

            echo '<td>'.$no.'.</td><td class="show">'.$record['productname'].'</td>
            <td><img src="../main/images/'.$record['image'].'" alt="'.$record['productname'].'" width="100" /></td>
            <td class="description">'.$record['description'].'</td>
            <td>'.$record['price'].'</td>';

            echo '<td>
           <a href="index.php?op=delete_product&amp;id_product='.$record['id_product'].'">delete</a> |
           <a href="index.php?op=update_product&amp;id_product='.$record['id_product'].'">update</a>
           </td>';
            echo "</tr>";
            $no++;
        }
        echo '</table>';
        mysqli_free_result($result);
    }
    ?>
</div>