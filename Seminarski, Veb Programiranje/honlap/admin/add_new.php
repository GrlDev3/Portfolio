<?php
define("VARIABLE","jrUmrlbHRevgwhskwpLdpoxkksqe");
require("../db_config.php");
require("functions.php");

$productname = "";
$image = "";
//$file ="";
$description = "";
$price = 0;

if(isset($_POST['sb']))
    $sendbutton=$_POST['sb'];

if(isset($_POST['productname']))
    $productname = mysqli_real_escape_string($connection, $_POST['productname']);

/*if(is_uploaded_file($_FILES['image'])){
    $image="set";
}*/

if(fileUploaded()){$image = "set";}

if(isset($_POST['description']))
    $description = mysqli_real_escape_string($connection, $_POST['description']);

if(isset($_POST['price']))
    $price = mysqli_real_escape_string($connection, $_POST['price']);


if(empty($productname) OR empty($image) OR empty($description) OR empty($price))
{
    header("Location:index.php?op=new&productname=$productname&description=$description&price=$price&message=error");
    exit();
}
else{
    $file = $_FILES['image']['name'];
    $file_loc = $_FILES['image']['tmp_name'];
   // $file_type = $_POST['image']['type'];
    $folder="../main/images/";

    //a fajl nevet kis betukre irjuk at
    $new_file_name = strtolower($file);

    //esetleges helykozoket lecsereljuk - jelekre
    $final_file = str_replace(' ','-',$new_file_name);

    if(move_uploaded_file($file_loc,$folder.$final_file)) {

        $sql = "INSERT INTO products (productname,image,description,price) VALUES ('$productname','$final_file','$description','$price')";

        $result = mysqli_query($connection, $sql) or die(mysqli_error($connection));

        mysqli_free_result($result);
        mysqli_close($connection);

        header("Location:index.php?op=new&productname=$productname&description=$description&price=$price&message=ok");
        exit();
    }
}
?>