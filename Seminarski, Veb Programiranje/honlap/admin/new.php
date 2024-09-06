<div>
    <h1>Add a new product</h1>
<?php
$productname="";
$description="";
$price=0;

if(isset($_GET['message']) AND $_GET['message']=="error"){
    $productname=$_GET["productname"];
    $description=$_GET['description'];
    $price=$_GET['price'];
    echo "You must fill all fields and choose an image to upload!";
}

if (isset($_GET['message']) AND $_GET['message']=="ok"){
    echo "A new product was successfully entered!";
}

?>
    <form action="add_new.php" method="post" enctype="multipart/form-data">
        <label for="productname">Name of the product: </label><input value="<?php echo $productname; ?>" type="text" name="productname" id="productname" /><br /><br />
        <label for="image">Image: </label><input id="image" type="file" name="image" /><br /><br />
        <label for="description">Description: </label><br /><textarea id="description" maxlength="100" cols="20" rows="5" name="description" ><?php echo $description; ?></textarea><br /><br />
        <label for="price">Price: </label><input type="number" name="price" id="price" value="<?php echo $price; ?>"/><br /><br />
        <input class="btn" type="submit" name="sb" value="save"/>
        <input class="btn" type="reset" name="rb" value="cancel"/>

    </form>
</div>

