import FreeCAD, Part, Mesh, os
import sys

def convert_stl_to_step(folder):
    # Initialize lists for STL and STEP files
    stlfiles = []
    stepfiles = []

    # Check for filenames that contain .stl
    for f in os.listdir(folder):
        if f.upper().endswith(".STL"):
            stlfiles.append(folder + os.sep + f)

    if not stlfiles:
        print("No STL files found in the given folder")
        os._exit(0)

    # Create a new FreeCAD document
    doc = FreeCAD.newDocument()

    for stlfile in stlfiles:
        try:
            print(f"Processing {stlfile}...")
            # Open the STL file
            mesh = Mesh.Mesh(stlfile)

            # Get the base name of the file (without extension) for naming purposes
            basename = os.path.splitext(os.path.basename(stlfile))[0]

            # Create shape from mesh
            shape_obj_name = f"{basename}_Shape"
            refined_shape_obj_name = f"{basename}_RefinedShape"
            solid_obj_name = f"{basename}_Solid"
            stepfile = folder + os.sep + f"{basename}.step"

            stepfiles.append(stepfile)

            # Add a new part feature to the document
            shape_obj = doc.addObject('Part::Feature', shape_obj_name)

            # Convert the mesh to a shape
            __shape__ = Part.Shape()
            __shape__.makeShapeFromMesh(mesh.Topology, 0.010000, True)
            if __shape__.isNull():
                print(f"Failed to create shape from mesh for {stlfile}")
                continue
            shape_obj.Shape = __shape__
            shape_obj.purgeTouched()
            del __shape__

            # Refine the shape
            refined_shape = doc.addObject('Part::Refine', refined_shape_obj_name)
            refined_shape.Source = shape_obj
            refined_shape.Label = shape_obj.Label
            shape_obj.Visibility = False

            # Recompute the doc to apply changes
            doc.recompute()

            # Convert to solid
            __s__ = refined_shape.Shape
            if __s__.isNull():
                print(f"Failed to refine shape for {stlfile}")
                continue

            __s__ = Part.Solid(__s__)
            if __s__.isNull():
                print(f"Failed to create solid from shape for {stlfile}")
                continue

            solid = doc.addObject("Part::Feature", solid_obj_name)
            solid.Label = f"{basename} (Solid)"
            solid.Shape = __s__

            # Export to STEP file
            __objs__ = [solid]
            Part.export(__objs__, stepfile)
            print(f"Exported {stepfile}...")
            del __objs__

            # Clear objects from the document
            doc.removeObject(shape_obj.Name)
            doc.removeObject(refined_shape.Name)
            doc.removeObject(solid.Name)

        except Exception as e:
            print(f"Error processing {stlfile}: {e}")
            continue

    # Recompute the document once after all modifications
    doc.recompute()

    # Inform the user that the process is complete
    print(f"Conversion completed. {len(stepfiles)} STEP files created in {folder}.")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: FreeCAD --console --run convert_stl_to_stp.py <folder>")
        sys.exit(1)

    folder = sys.argv[1]
    convert_stl_to_step(folder)

