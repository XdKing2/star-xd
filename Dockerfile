FROM node:20-bullseye-slim

WORKDIR /root/star-xd

# Install minimal dependencies (sharp doesn't need to build)
RUN apt-get update && apt-get install -y \
    ffmpeg \
    imagemagick \
    git \
    python3 \
    make \
    g++ \
    libvips-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy package files
COPY package.json package-lock.json ./

# Install without building from source (use prebuilt binaries)
RUN npm install --legacy-peer-deps

# Copy loader
COPY index.js ./

# Create directories
RUN mkdir -p data session temp assets lib plugins

EXPOSE 5000

CMD ["npm", "run", "start:optimized"]
