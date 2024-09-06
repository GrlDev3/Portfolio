<?php
include ('../db_config.php');
session_start();

$user_check = $_SESSION['username'];
$ses_sql = mysqli_query($connection, "SELECT username FROM people WHERE username='$user_check'");
$row = mysqli_fetch_array($ses_sql,MYSQLI_ASSOC);

$login_user = $row['username'];
if(!isset($user_check)){
    header("Location:../main");
}