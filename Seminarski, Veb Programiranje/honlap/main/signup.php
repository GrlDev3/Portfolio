<?php
//include ("../db_config.php");
$msg = "";
if(isset($_POST["signupsb"])) {

    $username = mysqli_real_escape_string($connection,$_POST["username"]);
    $email = mysqli_real_escape_string($connection,$_POST["email"]);
    $password = mysqli_real_escape_string($connection,$_POST["password"]);

    $md5pass = md5($password);

    $sql1 = "SELECT email FROM people WHERE email='$email'";
    $sql2 = "SELECT username FROM people WHERE username='$username'";
    $result1 = mysqli_query($connection,$sql1);
    $result2 = mysqli_query($connection,$sql2);
    $row1 = mysqli_fetch_array($result1,MYSQLI_ASSOC);
    $row2 = mysqli_fetch_array($result2,MYSQLI_ASSOC);
    if(mysqli_num_rows($result1) == 1){
        $msg = "Sorry... This email already exists...";
    }elseif(mysqli_num_rows($result2) == 1){
        $msg = "Sorry... This username already exists...";
    }
    else{
        $insql = "INSERT INTO people (username, email, password)
                  VALUES ('$username','$email', '$md5pass')";
        $query = mysqli_query($connection, $insql);
        if($query){
            $msg = "Thank you! You are now registered. You can login now.";
        }
    }
}
?>
<div>
    <script type="text/javascript" src="../js/functionsignup.js"></script>
    <h2><?php echo $msg; ?></h2>
    <div class="loginform" id="signupform">
        <div id="error"></div>
        <form method="post" name="signup" id="signup">
            <h2>Sign Up</h2>
            <label for="username">Username:</label><br/><input id="username" name="username" type="text" /><br /><br />
            <label for="email">Email:</label><br><input id="email" name="email" type="text" /><br /><br />
            <label for="password">Password:</label><br/><input id="password" name="password" type="password" /><br /><br />
            <input class="btn" name="signupsb" value="Sign Up" type="submit"/>
        </form>
    </div>
</div>