$fn=120;
difference(){
cylinder(14, 30, 6.5);
cylinder(14, 25, 1.5);
}
translate([0,0,-50])
difference(){
cylinder(50, 30, 30);
cylinder(50, 25, 25);
}
translate([0,0,14])
difference(){
cube([13, 13, 4], center=true);
cube([10, 10, 4], center=true);
}

translate([-30,0,-30])
difference(){
cube([60,30,30]);
translate([30,0,0])
cylinder(30,30,30);
}