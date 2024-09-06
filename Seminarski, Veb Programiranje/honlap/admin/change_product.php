<?php
define("VARIABLE","jrUmrlbHRevgwhskwpLdpoxkksqe");
require("../db_config.php");
require("functions.php");

$id_product = "";
$productname = "";
$description = "";
$price = "";

if(isset($_POST['id_product']))
    $id_product = (int)mysqli_real_escape_string($connection,$_POST['id_product']);

if(isset($_POST['productname']))
    $productname = mysqli_real_escape_string($connection,$_POST['productname']);

if(isset($_POST['description']))
    $description = mysqli_real_escape_string($connection,$_POST['description']);

if(isset($_POST['price']))
    $price = mysqli_real_escape_string($connection,$_POST['price']);

if(empty($id_product) OR empty($productname) OR empty($description) OR empty($price))
{
    header("Location:index.php?op=update_product&id_product=$id_product&message=error");
    exit();
}
else {
    if(fileUploaded()){
        $file = $_FILES['image']['name'];
        $file_loc = $_FILES['image']['tmp_name'];
        // $file_type = $_POST['image']['type'];
        $folder = "../main/images/";

        //a fajl nevet kis betukre irjuk at
        $new_file_name = strtolower($file);

        //esetleges helykozoket lecsereljuk - jelekre
        $final_file = str_replace(' ', '-', $new_file_name);

        if (move_uploaded_file($file_loc, $folder . $final_file)) {

            $sql = "UPDATE products SET productname='$productname',image='$final_file',description='$description',price='$price'
          WHERE id_product='$id_product'
          ";
            $result = mysqli_query($connection, $sql) or die(mysqli_error($connection));

            mysqli_free_result($result);
            mysqli_close($connection);

            header("Location:index.php?op=update_product&id_product=$id_product&message=ok");
            exit();

        }
    }else{
        $sql = "UPDATE products SET productname='$productname',description='$description',price='$price'
          WHERE id_product='$id_product'
          ";
        $result = mysqli_query($connection, $sql) or die(mysqli_error($connection));

        mysqli_free_result($result);
        mysqli_close($connection);

        header("Location:index.php?op=update_product&id_product=$id_product&message=ok");
        exit();
    }

}
?>