<div>
    <h1>List of products - for update</h1>
    <?php
    $sql = "SELECT * FROM products ORDER BY productname ASC";

    $result = mysqli_query($connection,$sql) or die(mysqli_error($connection));

    if(mysqli_num_rows($result)>0)
    {
        echo '<form method="get">';
        echo "<label>choose product:</label>";
        echo ' <select name="id_product">';
        echo '<option value="choose">choose</option>';

        while ($record=mysqli_fetch_array($result,MYSQLI_BOTH))
        {
            echo '<option value="'.$record['id_product'].'">'.$record['productname'].'</option>';
        }

        echo '</select>';
        echo ' <input type="hidden" name="op" value="update_product" />';
        echo '<br /><br /> <input class="btn" type="submit" value="send" />';
        echo '</form>';

        mysqli_free_result($result);
    }
    ?>
</div>