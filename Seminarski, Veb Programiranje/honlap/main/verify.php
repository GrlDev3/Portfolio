<?php
session_start();
//include ("../db_config.php"); //az adatbazissal valo kapcsolat felallitasa

$error = ""; //valtozo a hibauzenetek tarolasara
if(isset($_POST["loginsb"])){
    if(empty($_POST["username"]) || empty($_POST["password"])){
        $error = "Both fields are required.";
        //header("Location:index.php?op=login&error=$error");
    }else{
        //definialjuk a $username es $password valtozokat, vedve az adatbazist
        $username =mysqli_real_escape_string($connection, $_POST['username']);
        //echo $username;
        $password =mysqli_real_escape_string($connection, $_POST['password']);
        //echo $password;
        $md5pass = md5($password);
        //echo $md5pass;

        //leellenorizzuk, hogy a felhasznalonev es a jelszo az adatbazisban van-e
        $sql = "SELECT id_user FROM people WHERE username='$username' and password='$md5pass'";
        $result = mysqli_query($connection,$sql);
        $row = mysqli_fetch_array($result,MYSQLI_ASSOC);

        //Ha megtalalhato a felhasznalonev es a jelszo az adatbazisunkban akkor letrehozzunk egy sessiont.
        //Kulonben kiiratjuk echoval a hibauzenetet(error).

        if(mysqli_num_rows($result) == 1){
            $_SESSION['username'] = $username; //Session inicializalasa
            header("Location:../admin"); //atiranyitas egy masik oldalra
        }else{
            $error = "Incorrect username or password.";
            //header("Location:index.php?op=login&error=$error");
        }

    }
}

?>

