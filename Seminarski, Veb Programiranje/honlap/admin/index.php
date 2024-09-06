<?php
//session_start();
define("VARIABLE","jrUmrlbHRevgwhskwpLdpoxkksqe");

include("check.php");
require("functions.php");
?>
<!DOCTYPE html
    PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
    <title>Administration of Avon</title>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
    <meta http-equiv="content-language" content="en"/>
    <link type="text/css" rel="stylesheet" href="../css/adminstyle.css"/>
    <script type="text/javascript" src="https://ajax.googleapis.com/ajax/libs/jquery/1.12.2/jquery.min.js"></script>
    <script type="text/javascript" src="../js/jquery.js"></script>
    <script type="text/javascript" src="../js/admin.js"></script>
</head>
<body>
<div id="header">
    <h1>Administration of Avon Cosmetics</h1>
</div>
<div id="navigation">
    <ol>
        <li><a href="index.php">Home</a></li>
        <li><a href="index.php?op=new">New</a></li>
        <li><a href="index.php?op=list">List</a></li>
        <li><a href="index.php?op=update">Update</a></li>
        <li><a href="index.php?op=delete">Delete</a></li>
        <li><a href="index.php?op=logout">Logout</a></li>
        <li id="user" >Hello <?php echo $_SESSION['username'];?>!</li>
    </ol>
</div>
<?php
$op = "home";

if(isset($_GET['op']))
    $op = mysqli_real_escape_string($connection,$_GET['op']);

switch($op)
{
    case "home";
    default: include("home.xhtml");
        break;

    case "new": include("new.php");
        break;

    case "list": include("list.php");
        break;

    case "logout": include("logout.php");
        break;

    case "update": include("update.php");
        break;

    case "delete": include("delete.php");
        break;

    case "delete_product":
        if(isset($_GET['id_product'])){
            $id_product = (int) mysqli_real_escape_string($connection, $_GET['id_product']);
            deleteProduct($id_product);
        }
        include("list.php");
        break;

    case "update_product":
        if(isset($_GET['id_product'])) {
            if ($_GET['id_product']!="choose") {
                $id_product = (int)mysqli_real_escape_string($connection, $_GET['id_product']);
                include("update_product.php");
            }
            else
                include("update.php");
        }
        break;

}


?>

</body>
</html>
