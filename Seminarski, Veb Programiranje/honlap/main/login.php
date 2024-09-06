<?php
include ('verify.php'); //beagyazzuk az ellenorzo fajlt
if (isset($_SESSION['username']) != ''){
    header('Location:../admin');
}

?>
<div id="loginform" class="loginform">
    <script type="text/javascript" src="../js/functionlogin.js"></script>
    <div id="error"><?php echo $error; ?></div><br/>
<form id="login" name="login" method="post">
    <h2>Login</h2>
    <label for="username">Username:</label><br/><input id="username" name="username" type="text" /><br><br>
    <label for="password">Password:</label><br/><input id="password" name="password" type="password" /><br><br>
    <input class="btn" name="loginsb" value="Login" type="submit"/>
</form>
</div>

