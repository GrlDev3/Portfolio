<div>
    <h1>Update product</h1>
    <?php
    $folder="../main/images/";
    $sql = "SELECT * FROM products WHERE id_product='$id_product'";

    $result = mysqli_query($connection,$sql) or die(mysqli_error($connection));

    if(mysqli_num_rows($result)>0)
    {

        while ($record=mysqli_fetch_array($result,MYSQLI_BOTH))
        {
            $productname = $record['productname'];
            $image = $record['image'];
            $description = $record['description'];
            $price = $record['price'];
        }


        if(isset($_GET['message']) AND $_GET['message']=="error")
            echo "You must fill all fields!<br />";

        if(isset($_GET['message']) AND $_GET['message']=="ok")
            echo "Product was successfully updated!<br />";

        mysqli_free_result($result);
        echo '
    <form method="post" action="change_product.php" enctype="multipart/form-data">
    <label>Name of a product: </label><input type="text" name="productname" value="'.$productname.'" /><br /><br />
    <label>Image: </label><input type="file" name="image"/><br />
    <label>Current image: </label><br><img src="'.$folder.$image.'" alt="'.$productname.'" width="100"/><br><br />
    <label>Description: </label><br /><textarea id="description" maxlength="100" cols="20" rows="5" name="description" >'.$description.'</textarea><br /><br />
    <label>Price: </label><input type="text" name="price" value="'.$price.'" /><br />
    <input type="hidden" name="id_product" value="'.$id_product.'" />
    <br /> <input class="btn" type="submit" value="change" name="sb" />
     <input class="btn" type="reset" value="cancel" name="rb" />
    </form>
    ';

    }
    else
        echo "No data for this id!";
    ?>
</div>