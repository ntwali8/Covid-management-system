<?php
$conn = new mysqli('localhost', 'root', '', 'covid_19');

if ($conn->connect_error) {
    die("connection failed");
}
// Getting a list of all districts from database
$sql = "Select * from districts";
$districts = [];
if($result = $conn->query($sql)) {
    while ($row = $result->fetch_assoc()) {
        array_push($districts, $row['name']);
    }
    //free result set
    $result->free();
}
//Opening files depending on the district lists from database
for ($x = 0; $x < count($districts); $x++) {
    $districts[$x] .= ".txt";

    if (file_exists($districts[$x])){
        $file = fopen($districts[$x], 'r');
        while (!feof($file)) {
            $content = fgets($file);
            $content_array = explode(" ", $content);
            if ($content_array[0] != "") {
                list($lname,$fname,$date,$gender,$health_condition,$hoFname, $hoLname) = $content_array;
    
                //get healthofficer id
                $health_officer = $hoFname;
                $health_officer .= "";
                $health_officer .= $hoLname;
    
                $health_officer = str_replace("\n", "", $health_officer);// remove new line character
    
                $ho_sql = "SELECT `id`, `hospital_id` FROM `healthworkers` WHERE `username` = '$health_officer'";
                if($results = $conn->query($ho_sql)) {
                    if ($results->num_rows > 0) {
                        while ($rowId = $results->fetch_assoc()) {
                            $hoId = $rowId["id"];
                        }
                    }
                    else {
                        print_r("no records for health officer");
                    }
                    //free result set
                    $results->free();
                }
                else {
                    print_r("incorrect query");
                }
    
                //combine name for patient
                $patientname = $lname;
                $patientname .= " ";
                $patientname .= $fname;
    
                //use integers for the health condition
                if ($health_condition == "A") {
                    $health_condition = 1;
                }
                else {
                    $health_condition = 2;
                }
    
                //add a timestamp to the date
                $date .= date(" H:i:s");
    
                //insert data into database
                $result = $conn->query("INSERT INTO `patients`(`name`, `asymptomatic`, `gender`, `healthWorker_id`, `created_at`, `updated_at`) VALUES ('$patientname', '$health_condition', '$gender', '$hoId', '$date', '$date')");
                $result = $conn->query("UPDATE `healthworkers` SET `patientNo`=patientNo+1 WHERE `username` = '$health_officer'");
                if ($result) {
                    print_r ("Successfully uploaded");
                }
                else{
                    print_r ("Didn't upload");
                }
            }
        }
        fclose($file);
        //reset file
        $reset = fopen($districts[$x], 'w');
        fclose($reset);
    }
    // else {
    //     print_r("file not found");
    // }
}
?>
