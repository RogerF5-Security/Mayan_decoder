# Mayan Decoder

Mayan Decoder es una aplicacion externa `.fap` para Flipper Zero que convierte un numero decimal a numeracion maya usando la API de Canvas.

## Funcionalidad

- Rango inicial: `0` a `999`.
- Conversion a base vigesimal para representar niveles mayas apilados verticalmente.
- `0`: concha vectorial simple.
- `1..4`: puntos.
- `5..19`: barras horizontales de cinco unidades y puntos residuales.
- Renderizado directo con `ViewPort` y `Canvas`.

## Controles

- `Up` / `Right`: incrementar el numero.
- `Down` / `Left`: decrementar el numero.
- `OK` / `Back`: salir de la aplicacion.

## Estructura

```text
.
+-- application.fam
+-- mayan_decoder.c
+-- assets
|   +-- generate_icon.py
|   +-- icon.png
+-- README.md
```

## Compilacion con uFBT

Instalar uFBT en Windows:

```powershell
py -m pip install --upgrade ufbt
```

Compilar desde la raiz del proyecto:

```powershell
cd "C:\Users\R0G3R\Documents\Tools\Flipper Zero\N#Maya"
ufbt
```

El binario `.fap` se generara dentro del directorio `dist`.

Compilar, subir y ejecutar en un Flipper conectado por USB:

```powershell
ufbt launch
```

Actualizar el SDK usado por uFBT:

```powershell
ufbt update
```

## Regenerar el icono

El icono FAP debe ser PNG monocromatico de 1 bit y `10x10` pixeles. Para regenerarlo:

```powershell
py .\assets\generate_icon.py
```

## Git

Repositorio remoto configurado:

```text
https://github.com/RogerF5-Security/Mayan_decoder
```

Primer push recomendado:

```powershell
git push -u origin main
```
