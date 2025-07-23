import javax.imageio.ImageIO;
import javax.swing.*;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

public class PIMConverter {
    
    // Paleta de 9 colores (RGB)
    private static final Color[] PALETTE = {
        new Color(0, 0, 0),       // n - Negro
        new Color(0, 0, 170),     // b - Azul
        new Color(0, 170, 0),     // g - Verde
        new Color(0, 170, 170),   // c - Cyan
        new Color(170, 0, 0),     // r - Rojo
        new Color(170, 0, 170),   // m - Magenta
        new Color(170, 85, 0),    // l - Marrón
        new Color(170, 170, 0),   // y - Amarillo
        new Color(170, 170, 170)  // w - Blanco
    };
    
    // Caracteres correspondientes a cada color
    private static final char[] COLOR_CHARS = {'n', 'b', 'g', 'c', 'r', 'm', 'l', 'y', 'w'};

    public static void main(String[] args) {
        JFileChooser fileChooser = new JFileChooser();
        fileChooser.setDialogTitle("Selecciona una imagen PNG");
        
        if (fileChooser.showOpenDialog(null) == JFileChooser.APPROVE_OPTION) {
            File inputFile = fileChooser.getSelectedFile();
            File outputFile = new File(inputFile.getParent(), 
                                     inputFile.getName().replace(".png", ".pim"));
            
            try {
                convertPNGtoPIM(inputFile, outputFile);
                JOptionPane.showMessageDialog(null, 
                    "Conversión completada!\nArchivo guardado como: " + outputFile.getName(),
                    "Éxito", JOptionPane.INFORMATION_MESSAGE);
            } catch (IOException e) {
                JOptionPane.showMessageDialog(null, 
                    "Error al convertir la imagen: " + e.getMessage(),
                    "Error", JOptionPane.ERROR_MESSAGE);
            }
        }
    }
    
    public static void convertPNGtoPIM(File pngFile, File pimFile) throws IOException {
        BufferedImage image = ImageIO.read(pngFile);
        StringBuilder pimContent = new StringBuilder();
        
        // Cabecera del filesystem
        pimContent.append("n:").append(pimFile.getName()).append("\n");
        
        // Convertir imagen a formato PIM
        for (int y = 0; y < image.getHeight(); y++) {
            for (int x = 0; x < image.getWidth(); x++) {
                Color pixelColor = new Color(image.getRGB(x, y));
                char closestChar = findClosestColor(pixelColor);
                pimContent.append(closestChar);
            }
            
            // No agregar \n después de la última línea
            if (y < image.getHeight() - 1) {
                pimContent.append("\n");
            }
        }
        
        // Fin de imagen
        pimContent.append("\nq");
        
        // Calcular tamaño (sin contar cabeceras)
        int contentSize = pimContent.length() - pimContent.indexOf("\n") - 1;
        pimContent.insert(pimContent.indexOf("\n") + 1, "l:" + contentSize + "b\n");
        
        // Escribir archivo
        try (FileWriter writer = new FileWriter(pimFile)) {
            writer.write(pimContent.toString());
        }
    }
    
    private static char findClosestColor(Color color) {
        double minDistance = Double.MAX_VALUE;
        char closestChar = 'n';
        
        for (int i = 0; i < PALETTE.length; i++) {
            double distance = colorDistance(color, PALETTE[i]);
            if (distance < minDistance) {
                minDistance = distance;
                closestChar = COLOR_CHARS[i];
            }
        }
        
        return closestChar;
    }
    
    private static double colorDistance(Color c1, Color c2) {
        // Distancia euclidiana al cuadrado entre colores
        int redDiff = c1.getRed() - c2.getRed();
        int greenDiff = c1.getGreen() - c2.getGreen();
        int blueDiff = c1.getBlue() - c2.getBlue();
        
        return redDiff * redDiff + greenDiff * greenDiff + blueDiff * blueDiff;
    }
}