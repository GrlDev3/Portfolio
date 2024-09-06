<?php

function deleteProduct($id_product)
{
    global $connection;
    $sql = "DELETE FROM products WHERE id_product='$id_product'";
    $result = mysqli_query($connection,$sql) or die(mysqli_error($connection));
    // delete image of product http://php.net/manual/en/function.unlink.php

}

function fileUploaded()
{
    if(empty($_FILES)) {
        return false;
    }
    $file = $_FILES['image'];
    /*if(!file_exists($this->file['tmp_name']) || !is_uploaded_file($this->file['tmp_name'])){
        //$this->errors['FileNotExists'] = true;
        return false;
    }
    */
    if(!file_exists($_FILES['image']['tmp_name']) || !is_uploaded_file($_FILES['image']['tmp_name'])) {
        return false;
    }

    return true;
}

?>